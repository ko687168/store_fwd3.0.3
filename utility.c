/*
 * Part of Very Secure FTPd
 * Licence: GPL v2
 * Author: Chris Evans
 * utility.c
 */

#include "utility.h"
#include "sysutil.h"
#include "str.h"
#include "defs.h"

#define DIE_DEBUG

void
die(const char* p_text)
{
#ifdef DIE_DEBUG
  bug(p_text);
#endif
  vsf_sysutil_exit(2);
}

void
die2(const char* p_text1, const char* p_text2)
{
  struct mystr die_str = INIT_MYSTR;
  str_alloc_text(&die_str, p_text1);
  if (p_text2)
  {
    str_append_text(&die_str, p_text2);
  }
  else
  {
    str_append_text(&die_str, "(null)");
  }
  die(str_getbuf(&die_str));
}

/*process the PID */
void process_pid()
{
    FILE* file = NULL;
    char buf[128];
    long length = 0;
    size_t read_chars = 0;
    pid_t current_pid = 0;

    if (!tunable_pid_file)
    {
        return;
    }

    /* open in read binary mode */
    file = fopen(tunable_pid_file, "wb");
    if (file == NULL)
    {
        goto cleanup;
    }

    current_pid = getpid();
    length = sprintf(buf, "%ld", (size_t)current_pid);

    /* read the file into memory */
    read_chars = fwrite(buf, sizeof(char), (size_t)(length + 1), file);
    if ((long)read_chars != length + 1)
    {
        goto cleanup;
    }

cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return;
}

int execute_lock()
{
    int fd = -1;
    int ret = -1;
    if (!tunable_lock_file) {
        return 0;
    }
    // the file will keep open, until it is closed...
    fd = open(tunable_lock_file, O_RDONLY);
    if (fd < 0) {
        fd = open(tunable_lock_file, O_RDWR | O_CREAT, 0x777);
        if (fd < 0) {
            return -1;
        }
    }

    ret = flock(fd, LOCK_NB | LOCK_EX);
    return ret;
}

void
bug(const char* p_text)
{
  /* Rats. Try and write the reason to the network for diagnostics */
  vsf_sysutil_activate_noblock(VSFTP_COMMAND_FD);
  (void) vsf_sysutil_write_loop(VSFTP_COMMAND_FD, "500 OOPS: ", 10);
  (void) vsf_sysutil_write_loop(VSFTP_COMMAND_FD, p_text,
                                vsf_sysutil_strlen(p_text));
  (void) vsf_sysutil_write_loop(VSFTP_COMMAND_FD, "\r\n", 2);
  vsf_sysutil_exit(2);
}

void
vsf_exit(const char* p_text)
{
  (void) vsf_sysutil_write_loop(VSFTP_COMMAND_FD, p_text,
                                vsf_sysutil_strlen(p_text));
  vsf_sysutil_exit(0);
}

