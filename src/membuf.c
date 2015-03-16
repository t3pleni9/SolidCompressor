/*
 * delta.cpp
 * 
 * Copyright 2015 Justin Jose <justinjose999@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include "membuf.h"


mem_buf_t *mem_buf_new(size_t buf_len) {
    
    mem_buf_t *mf = (mem_buf_t *)malloc(sizeof(mem_buf_t));
    memset(mf, 0, sizeof(mem_buf_t));
   
    
    if (!(mf->buf = (char *)malloc(buf_len*sizeof(char)))) {
        perror("couldn't allocate instance of buffer");
    }
    
    if (!(mf->mem_in_buf = (char *)malloc(buf_len*sizeof(char)))) {
        perror("couldn't allocate instance of memory buffer");
    }
    
    if (!(mf->mem_out_buf = (char *)malloc(buf_len*sizeof(char)))) {
        perror("couldn't allocate instance of memory buffer");
    }
    
    memset(mf->buf, 0, buf_len);  
    memset(mf->mem_in_buf, 0, buf_len);  
    memset(mf->mem_out_buf, 0, buf_len);    
    mf->buf_len = buf_len;
    mf->mem_in_buf_len = buf_len;
    mf->mem_out_buf_len = 0;
    
    return mf;
}

void mem_buf_free(mem_buf_t *mb) {
    if(mb->buf)
        free(mb->buf);
    if(mb->mem_in_buf)
        free(mb->mem_in_buf);
    if(mb->mem_out_buf)
        free(mb->mem_out_buf);
    memset(mb, 0, sizeof *mb);
    free(mb);
}

rs_result mem_copy(void *arg, rs_long_t pos, size_t *len, void **buf)
{
    int        got;
    char       *memBuf = (char *) arg;

    if (!(memBuf + pos)) {
        perror("Null pointer encountered");
        return RS_MEM_ERROR;
    }

    memcpy(*buf, (memBuf + pos), *len);
    got = (*((char *)buf + *len) == *(memBuf + pos + *len) && *((char *)buf + *len));
    if (got == 0) {
        perror("Unexpected memory corruption");
        return RS_MEM_ERROR;
    } else {
        return RS_DONE;
    }
}

rs_result mem_fill(rs_job_t *job, rs_buffers_t *buf, void *opaque) {

    mem_buf_t *mb = (mem_buf_t *) opaque;
    int len;
    
    if (buf->next_in != NULL) {
        assert(buf->avail_in <= mb->buf_len);
        assert(buf->next_in >= mb->buf);
        assert(buf->next_in <= mb->buf + mb->buf_len);
    } else {
        assert(buf->avail_in == 0);
    }
    
    if (buf->eof_in ) {
        //buf->eof_in = 1;
        return RS_DONE;
    }
    
    if (buf->avail_in)
        /* Still some data remaining.  Perhaps we should read
           anyhow? */
        return RS_DONE;
    
    memcpy(mb->buf, mb->mem_in_buf, mb->buf_len);
    printf("BUF LEN: %d\n",mb->buf_len);
    len = (*(mb->mem_in_buf + mb->buf_len - 1) == *(mb->buf + mb->buf_len - 1));
    printf("lench %d %d\n", *(mb->mem_in_buf + mb->buf_len - 1), *(mb->buf + mb->buf_len - 1));
    if(!len) {
        perror("Unable to fill buf");
        return RS_MEM_ERROR;
    }       
    
    buf->avail_in = mb->buf_len;
    buf->next_in = mb->buf;
    buf->eof_in = 1;
    
    
    return RS_DONE;
}

rs_result mem_drain(rs_job_t *job, rs_buffers_t *buf, void *opaque) {
    int present;
    mem_buf_t *mb = (mem_buf_t *) opaque;
    
    if (buf->next_out == NULL) {
        assert(buf->avail_out == 0);
                
        buf->next_out = mb->buf;
        buf->avail_out = mb->buf_len;
                
        return RS_DONE;
    }
    
    assert(buf->avail_out <= mb->buf_len);
    assert(buf->next_out >= mb->buf);
    assert(buf->next_out <= mb->buf + mb->buf_len);

    present = buf->next_out - mb->buf;
    if (present > 0) {
        int result;
                
        assert(present > 0);

        if(mb->mem_out_buf_len + present > mb->mem_in_buf_len) {
            perror("couldn't allocate instance of memory buffer");
            return RS_MEM_ERROR;
        }
    
        
        memcpy((mb->mem_out_buf + mb->mem_out_buf_len), mb->buf, present);
        mb->mem_out_buf_len += present;
        printf("present = %d \n", present); 
        result = (*(mb->mem_out_buf + mb->mem_out_buf_len +  present) == *(mb->buf + present));
        if (!result) {
            perror("error draining buf to buf memory");
            return RS_MEM_ERROR;
        }
        buf->next_out = mb->buf;
        buf->avail_out = mb->buf_len;
        
    }
        
    return RS_DONE;
}
rs_result mem_run(rs_job_t *job, char *in_buffer, char *out_buffer, size_t in_len, size_t *out_len) {
    
    rs_buffers_t    buf;
    rs_result       result;
    mem_buf_t *in_mem_buf = NULL, *out_mem_buf = NULL;
    
    if(in_buffer)  {   
        in_mem_buf = mem_buf_new(in_len);
        memcpy(in_mem_buf->mem_in_buf, in_buffer, in_len);
        printf("buf end %d %d\n", *(in_mem_buf->mem_in_buf + in_len - 1), *(in_buffer + in_len - 1));
        
    }
    printf("LEN: %d\n",in_len); 
    if(out_buffer && out_len) {
        out_mem_buf = mem_buf_new(*out_len);
         
     }
    
    
    result = rs_job_drive(job, &buf, in_mem_buf? mem_fill:NULL, in_mem_buf, out_mem_buf?mem_drain:NULL, out_mem_buf);
    
    if(out_buffer && out_mem_buf->mem_out_buf) {
        
        memcpy(out_buffer, out_mem_buf->mem_out_buf, out_mem_buf->mem_out_buf_len);
        
        *out_len = out_mem_buf->mem_out_buf_len;
    }
    
    if (in_mem_buf)
        mem_buf_free(in_mem_buf);

    if (out_mem_buf)
        mem_buf_free(out_mem_buf);

    return result;
}

rs_result mem_loadsig(char *sig_buffer, rs_signature_t **sumset, rs_stats_t *stats, size_t sig_len)
{
    rs_job_t            *job;
    rs_result           r;
    size_t temp = 0;

    job = rs_loadsig_begin(sumset);
    
    r = mem_run(job, sig_buffer, NULL, sig_len, &temp);
    
    if (stats)
        memcpy(stats, rs_job_statistics(job), sizeof *stats);
    rs_job_free(job);

    return r;
}


rs_result mem_delta(rs_signature_t *sig, char *new_buffer, char *delta_buffer, rs_stats_t *stats, size_t new_len, size_t *delta_len)
{
    rs_job_t            *job;
    rs_result           r;

    job = rs_delta_begin(sig);

    r = mem_run(job, new_buffer, delta_buffer, new_len, delta_len);

    if (stats)
        memcpy(stats, rs_job_statistics(job), sizeof *stats);

    rs_job_free(job);

    return r;
}

rs_result mem_patch(char *basis_buffer, char *delta_buffer, char *new_buffer,
                        rs_stats_t *stats, size_t basis_len, size_t delta_len, size_t *new_len)
{
    rs_job_t            *job;
    rs_result           r;

    job = rs_patch_begin(mem_copy, basis_buffer);

    r = mem_run(job, delta_buffer, new_buffer, delta_len, new_len);
    
    if (stats)
        memcpy(stats, rs_job_statistics(job), sizeof *stats);

    rs_job_free(job);

    return r;
}

rs_result mem_sig(char *basis_buffer, char *sig_buffer,  int basis_len, size_t *sig_len, rs_stats_t *stats) {

    rs_job_t *job;
    rs_result r;
    
    if(sig_len)
        job = rs_sig_begin(*sig_len, RS_DEFAULT_STRONG_LEN);
    else 
        return RS_INTERNAL_ERROR;
    r = mem_run(job, basis_buffer, sig_buffer, basis_len, sig_len );
    if(stats) 
        memcpy(stats, rs_job_statistics(job), sizeof *stats);
    rs_job_free(job);

    return r;
}
