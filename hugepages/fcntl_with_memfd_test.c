#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/ioctl.h>

#define LOG(format, ...) \
  printf("%s:%d - " format "\n", __FILE__,__LINE__, ##__VA_ARGS__)

#define DIE_OR_ASSIGN(result, expr) \
  errno = 0; int result = expr; if(result < 0) { LOG("[%d: %s] " #expr " < 0", errno, strerror(errno)); exit(1); }

int
buffer_to_sealed_memfd_or_tmpfile (const char  *name,
                                   const char  *content,
                                   size_t       len)
{
  LOG("open file '%s'", name);
  DIE_OR_ASSIGN(memfd, memfd_create (name, MFD_CLOEXEC | MFD_ALLOW_SEALING));
  DIE_OR_ASSIGN(ftrunc_res, ftruncate (memfd, len));
  DIE_OR_ASSIGN(write_res, write (memfd, content, len));
  DIE_OR_ASSIGN(lseek_res, lseek (memfd, 0, SEEK_SET));
  DIE_OR_ASSIGN(fcntl_res, fcntl (memfd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_GROW | F_SEAL_WRITE | F_SEAL_SEAL));
  return memfd;
}

// Reproduces the error in https://github.com/flatpak/flatpak/issues/3409
// For flatpak routine at https://github.com/flatpak/flatpak/blob/master/common/flatpak-utils.c
int main(void) {
  const char* fname = "banana";
  const char content[] = "mr_monkey_banana_secret_stash";
  const uint content_len = sizeof(content);
  
  #define times 1
  int fds[times] = {};
  for(int i=0; i<times; ++i) {
    fds[i] = buffer_to_sealed_memfd_or_tmpfile(fname, content, content_len);
  }

  LOG("Process PID : '%d'", getpid());
  LOG("Press something to continue");
  getchar();

  for(int i=0; i<times; ++i) {
    LOG("close file_desc %d", fds[i]);
    DIE_OR_ASSIGN(close_res, close (fds[i]));
  }
  return 0;
}

