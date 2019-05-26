import os, sys, array, re, struct, collections

PAGE_4K = 4096
PAGE_2M = 2 * 1024**2
MAX_MEM = 32 * 1024**3
PFN_LEN = 8
PFN_FLG_LEN = 8

class Vma(collections.namedtuple('_Vma', ['start', 'end', 'map'])):
  sep_rx = re.compile('\s+')
  bounds_rx = re.compile('^([0-9a-f]+)-([0-9a-f]+)')
  
  @staticmethod
  def FromTxt(line):
    tokens = Vma.sep_rx.split(line)
    assert len(tokens) >= 5
    bounds = Vma.bounds_rx.search(tokens[0])
    return Vma(int(bounds.group(1), 16),
               int(bounds.group(2), 16),
               len(tokens) > 5 and not tokens[5].startswith('[') and tokens[5])
        

# https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html?highlight=pagemap
class PageFrame(collections.namedtuple('_PageFrame', ['pfn', 'vpage', 'flags'])):
  def is_present(self): return (self.pfn >> 63) % 2 
  def is_filemap(self): return (self.pfn >> 61) % 2 
  def is_exclmap(self): return (self.pfn >> 56) % 2 
  def is_sfdirty(self): return (self.pfn >> 55) % 2 

  def is_mmap(self): return (self.flags >> 11) % 2 
  def is_anon(self): return (self.flags >> 12) % 2 
  def is_head(self): return (self.flags >> 15) % 2 
  def is_tail(self): return (self.flags >> 16) % 2 
  def is_huge(self): return (self.flags >> 17) % 2 
  def is_none(self): return (self.flags >> 20) % 2 
  def is_trhp(self): return (self.flags >> 22) % 2 

  def addr(self): return self.pfn & 0x000fffffffffffff


def scan_vma_for_pid(pid):
  virtual_ranges = []
  with open('/proc/%d/maps' % pid, 'r') as fileobj:
   assert fileobj.readable() and fileobj.seekable() 
   for line in fileobj:
     if line:
       virtual_ranges.append( Vma.FromTxt(line) )
  virtual_ranges.sort()
  return virtual_ranges


def get_pfn_for_virtual_ranges(pid, virtual_ranges):
  virt_to_phy_map = collections.OrderedDict()
  buf = bytearray(b'\x00' * PFN_LEN)

  with open('/proc/%d/pagemap' % pid, 'rb') as fileobj:
    assert fileobj.readable() and fileobj.seekable() 
    for vma in virtual_ranges:
      frames = []
      fileobj.seek(PFN_LEN * vma.start//PAGE_4K, os.SEEK_SET)

      for page in range(vma.start, vma.end, PAGE_4K):
        fileobj.readinto(buf)
        pfn = struct.unpack('=Q', buf)[0]
        frames.append(PageFrame(pfn, page, None))
      virt_to_phy_map[vma] = frames
  return virt_to_phy_map


def add_flags_to_phy_pages(virt_to_phy_map):
  buf = bytearray(b'\x00' * PFN_FLG_LEN)
  with open('/proc/kpageflags', 'rb') as fileobj:
    assert fileobj.readable() and fileobj.seekable() 

    for vma,frames in virt_to_phy_map.items():
      for idx,pfn in enumerate(frames):
        fileobj.seek(PFN_FLG_LEN * pfn.addr(), os.SEEK_SET)
        fileobj.readinto(buf)
        flags = struct.unpack('=Q', buf)[0]
        new_pfn = PageFrame(pfn.pfn, pfn.vpage, flags)
        # wtf ?! pfn are higher than installed memory ?
        #assert not new_pfn.is_none() and new_pfn.addr() * PAGE_4K < MAX_MEM, \
        #  "%x, %x <? %x" % (new_pfn.pfn, new_pfn.addr() * PAGE_4K, MAX_MEM)
        frames[idx] = new_pfn
  return virt_to_phy_map  


def display_huge_page_info(virt_to_phy_map):
  to_str = lambda p: "paddr:%14x, vaddr:%14x, raw:%8x, is_mmap:%d, is_anon:%d, is_huge:%d, is_trhp:%d, is_head:%d, is_tail:%d" % (
    p.addr()*PAGE_4K, p.vpage, p.flags, p.is_mmap(), p.is_anon(), p.is_huge(), p.is_trhp(), p.is_head(), p.is_tail(),
  )
  for vma,frames in virt_to_phy_map.items():
    print(vma)
    for pfn in frames:
      #if pfn.is_present() and pfn.is_anon():
      if pfn.is_present() and (pfn.is_huge() or pfn.is_trhp() or pfn.is_head() or pfn.is_tail()):
        print(to_str(pfn))
    print()


def main (args):
  pid = int(args[1]) #os.getpid()
  virtual_ranges = scan_vma_for_pid(pid)
  virt_to_phy_map = get_pfn_for_virtual_ranges(pid, virtual_ranges)
  add_flags_to_phy_pages(virt_to_phy_map)
  display_huge_page_info(virt_to_phy_map)


if __name__ == '__main__':
  main(sys.argv)

