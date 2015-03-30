#include "deduplib.h"
#include "diff.h"
#include "zd_mem.h" 
#include "zlib.h"


#include <stdlib.h>


int main_single(int argc, char *argv[]){
  FILE *fp_ref;
  zd_mem_buffer tar_buf;
  zd_mem_buffer ref_buf;
  Bytef *delta = NULL;
  uLong d_size = 0;

  /* open and check the input files */
  if( !(fp_ref = fopen(argv[1],"rb")) )
  {
    perror(argv[1]);
    exit(0);
  }

  if(argc>2 && (!freopen(argv[2],"rb",stdin)))
  {
    perror(argv[2]); 
    exit(0);
  }

  /* copy input data to memory */
  if(dread_file(fp_ref,&ref_buf) < 0) perror(argv[1]);
  if(dread_file(stdin,&tar_buf) < 0) perror(argv[2]);

  /* compress the data */
  if(zd_compress1((const Bytef*) ref_buf.buffer, ref_buf.pos - ref_buf.buffer,
		  (const Bytef*) tar_buf.buffer, tar_buf.pos - tar_buf.buffer,
		  &delta, &d_size) == ZD_OK){

    /* successfull compression write the delta to a file */
    if(argc>3 && (!freopen(argv[3],"wb",stdout))){
      perror(argv[3]);
      exit(0);
    }
    if(d_size != fwrite(delta, sizeof(char), d_size, stdout)) perror("ouput");
  }
  
  /* release memory */
  free(delta);
  zd_free(&tar_buf);
  zd_free(&ref_buf);

  /* close files */
  fclose(fp_ref);

  return 0;
}

int main(int argc, char **argv) {
        
       int i;

  if(argc<2 || argc>(REFNUM+3))   /* check command line parameters */
  {
    //fprintf(stderr,"%s",usage_single);
    if(REFNUM>1){
      //fprintf(stderr," |\n%s",usage_multi);
      //for(i=3;i<=REFNUM;++i) fprintf (stderr," %s%d", "[Reference",i);
      //for(i=3;i<=REFNUM;++i) fprintf (stderr,"%s", "]");
      //fprintf(stderr," %s\n",usage_multi_end);
    }
    else{
      fprintf(stderr,"\n");
    }
    exit(0);
  }

  if(argc <= 4){
    return main_single(argc,argv);
  }
  else{
   //return MainMulti(argc, argv);
  }
}
