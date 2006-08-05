
#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "zlib/zlib.h"

#include <sys/types.h>
#include <sys/stat.h>

static int fputlong (FILE* f, unsigned long x)
{
  int n;
  for (n = 0; n < 4; n++) {
    if (fputc((int)(x & 0xff), f) == EOF) return -1;
    x >>= 8;
  }
  return 0;
}

time_t get_mtime(FILE* f)
{
  struct stat s;

  if (fstat(fileno(f), &s) == 0) return s.st_mtime;
  else return 0;
}

FILE* optimal_gzip(FILE* ffin, const char* fout, size_t blocksize)
{
  time_t mtime = get_mtime(ffin);
  FILE* ffout = fopen(fout,"wb+");

  if (!ffout) { perror("open"); return NULL; }

  fwrite("\x1f\x8b\x08\x00",4,1,ffout);
  fputlong(ffout,mtime);
  fwrite("\x00\x03",2,1,ffout);
  {
    z_stream zs;
    unsigned char* inbuf = malloc(blocksize);
    unsigned char* outbuf = malloc(blocksize+500);
    int err, r;
    unsigned long crc = crc32(0L, Z_NULL, 0);
    
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = NULL;
    zs.total_in = 0;
    zs.total_out = 0;

    /* windowBits is passed < 0 to suppress zlib header */
    err = deflateInit2(&zs, 9,
		       Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

    for (r = 1; r > 0;) {
      r = fread(inbuf, 1, blocksize, ffin);

      if (r < 0) break;

      crc = crc32(crc, inbuf, r);

      zs.next_in = inbuf;
      zs.avail_in = r;
      zs.next_out = outbuf;
      zs.avail_out = blocksize+500;

      err = deflate(&zs, r ? Z_PARTIAL_FLUSH : Z_FINISH);
      switch (err) {
      case Z_STREAM_END:
      case Z_OK:
	{
	  int w = zs.next_out - outbuf;
	  
	  if (w != fwrite(outbuf, 1, w, ffout)) { perror("write"); r = -1; }
	}
	break;
      default:
	fprintf(stderr,"zlib error: %s (%d)\n",zs.msg, err);
	r = -1;
      }
    }

    fputlong(ffout, crc);
    fputlong(ffout, zs.total_in);
    fflush(ffout);
    free(outbuf);
    free(inbuf);
    if (fclose(ffin) != 0 || r != 0) { fclose(ffout); return NULL; }
  }
  rewind(ffout);
  return ffout;
}