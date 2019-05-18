# What is needed to allocate huge pages

## Memory allocation

```sh
pushd libhugetlbfs-2.20/tests/obj64
echo '######## mmap'           && ./mmap-gettest 1 10
echo '######## shmget'         && ./shm-gettest 1 10
echo '######## get_huge_pages' && ./get_huge_pages
echo '######## malloc'         && hugectl --heap malloc

pushd libhugetlbfs-2.20/tests
python2 run_tests.py
```

### No hugetlbfs mounted, no pages allocated, thp disabled

```sh
mount | grep huge
grep -i huge /proc/meminfo
cat_burst /sys/kernel/mm/hugepages/*/*
```

* mmap :           `Bad configuration: Unable to open temp file in hugetlbfs (Success)`
* shmget :         `Bad configuration: Do not have permission to use SHM_HUGETLB`
* get_huge_pages : `Bad configuration: Must have at least 4 free hugepages`
* malloc         : `WARNING: New heap segment map at 0x55b7ccc00000 failed: Cannot allocate memory / FAIL    Address is not hugepage`

### hugetlbfs mounted, no pages allocated, thp disabled

`sudo hugeadm --create-global-mounts`

* mmap :           `Failed to mmap the hugetlb file: Cannot allocate memory`
* shmget :         `Bad configuration: Do not have permission to use SHM_HUGETLB`
* get_huge_pages : `Bad configuration: Must have at least 4 free hugepages`
* malloc         : `WARNING: New heap segment map at 0x55b7ccc00000 failed: Cannot allocate memory / FAIL    Address is not hugepage`

### hugetlbfs mounted, pages allocated, thp disabled

```sh
sudo hugeadm --pool-pages-max DEFAULT:1G --pool-pages-min DEFAULT:128M --set-recommended-shmmax --set-recommended-min_free_kbytes
cat /proc/sys/vm/min_free_kbytes
cat /proc/sys/kernel/shmmax
```

* mmap :           `PASS`
* shmget :         `Bad configuration: Do not have permission to use SHM_HUGETLB`
* get_huge_pages : `PASS`
* malloc         : `PASS`

```sh
sudo hugeadm --set-shm-group 1001
cat /proc/sys/vm/hugetlb_shm_group
```

* shmget : `PASS`

## transparent huge pages

```sh
sudo hugeadm --thp-madvise

cat_burst /sys/kernel/mm/transparent_hugepage/*
cat_burst /sys/kernel/mm/transparent_hugepage/*/*
grep thp /proc/vmstat
zgrep HUGE /proc/config.gz
```

### no hugetlbfs mounted, no pages allocated, thp madvise

nothing is collapsed automatically

### hugetlbfs mounted, pages allocated, thp madvise

```
sudo hugeadm --thp-madvise
bin/malloc_hugepage_at_once memalign
hugectl --thp bin/malloc_hugepage_at_once
```

* nothing happens automatically
* memalign to 2M pages / hugectl --thp malloc
```
thp_fault_alloc 4
thp_zero_page_alloc 1
AnonHugePages:      8192 kB
thp_deferred_split_page 4 (after close)
```

> Note `hugectl --heap --force-preload malloc` will NOT use THP but libhugetlbfs custom allocation

### hugetlbfs mounted, pages allocated, thp always

`sudo hugeadm --thp-always`

```
AnonHugePages:     86016 kB
ShmemHugePages:        0 kB
HugePages_Total:      64
HugePages_Free:       64
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
Hugetlb:          131072 kB
thp_fault_alloc 699
thp_fault_fallback 0
thp_collapse_alloc 80
thp_collapse_alloc_failed 0
thp_file_alloc 0
thp_file_mapped 0
thp_split_page 0
thp_split_page_failed 0
thp_deferred_split_page 668
thp_split_pmd 129
thp_split_pud 0
thp_zero_page_alloc 1
thp_zero_page_alloc_failed 0
thp_swpout 0
thp_swpout_fallback 0
```

> Note transparent huge pages do not take from the allocated pool since the small pages are coalesced ?

* memalign to 2M pages / malloc **without madvise**
  * THP works (for unaligned malloc the head cannot be coalesced)


