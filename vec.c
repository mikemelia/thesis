/* VRANK pattern matching suite
 * Copyright (C) 1996 to Justin Zobel
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
 * See Copyright.txt for further details. */


/* Code for creating and accessing bitvectors.  They cannot be accessed
   until complete; once complete, they can only be extended by decoding,
   then recoding into a new vector. The code also assumes that the numbers
   to be encoded are at least one bit shorter than an integer. */

#include "def.h"
#include "vec.h"

static int put_n_bits_in_vector(BITVEC *, int, int);

static int get_n_bits_from_vector(BITVEC *, int);

static int put_bit(BITVEC *, int);

static int get_bit(BITVEC *);


/* Enlarge vector */
void expand_vector(BITVEC *vp) {
    char *newvec;
    int i;

    if (vp->size == 0) {
        fprintf(stderr, "Vector in inconsistent state.\n");
        exit(0);
    }
    else {
        /* should use realloc here */
        newvec = malloc(vp->size * 2 * sizeof(char));
        for (i = 0; i < vp->size; i++)
            newvec[i] = vp->vector[i];
        vp->size = vp->size * 2;
        free(vp->vector);
        vp->vector = newvec;
    }
}

/* Number of bits written to any bitvector; used for consistency check */
static int bitcount;

void initcmp() {
    bitcount = 0;
}


/* Return length of vector */
int veclen(BITVEC *vp) {
    return (vp->len);
}


/* Before a vector can be decoded, it must be reset */
void reset_vector(BITVEC *vp) {
    vp->last = -1;
    vp->pos = vp->bit = 0;
    vp->cur = vp->vector[vp->pos];
}


/* Initialising a vector before insertion of data */
void initialise_vector(BITVEC *vp) {
    vp->vector = (char *) malloc(INITVECLEN * sizeof(char));
    vp->size = INITVECLEN;
    vp->last = -1;
    vp->bit = vp->pos = vp->len = vp->cur = 0;
}


/* Write the contents of a vector to file.  Assumes that terminate_vector_for_writing has
   been called. */
void write_vector_to_file(BITVEC *vp, FILE *fp) {
#ifdef DEBUG
    int i;

    resetvec(vp);
    fprintf(stderr,"[", i);
    while( (i=getrunlen(vp)) >=0 )
	fprintf(stderr," %d", i);
    fprintf(stderr,"] {", i);
    resetvec(vp);
    while( (i=getdelta(vp)) >=0 )
	fprintf(stderr," %d", i);
    fprintf(stderr,"}", i);
#endif /* DEBUG */

    fwrite(&vp->len, sizeof(int), 1, fp);
    fwrite(vp->vector, sizeof(char), vp->len / 8 + 1, fp);
}


/* Read a vector back from file */
/* Returns -1 on error */
int read_vector_from_file(BITVEC *vp, FILE *fp) {
    int len;

    if (fread(&vp->len, sizeof(int), 1, fp) != 1)
        return (-1);
    /* WAS vp->size = vp->len/8 + 1; */
    vp->size = 4 * ((vp->len / 8 + 8) / 4); /* word boundaries */
    len = vp->len / 8 + 1;
    vp->vector = malloc(vp->size * sizeof(char));
    if (fread(vp->vector, sizeof(char), len, fp) != len)
        return (-1);
    reset_vector(vp);
    return (0);
}


/* Add the number n to the vector, assuming run-length coding */
int put_run_length(BITVEC *vp, int n) {
    int ret;

    if (n <= vp->last)
        return (0);        /* hack: don't add the same code twice */
    ret = put_delta(vp, n - vp->last);
    vp->last = n;
    return (ret);
}


/* Get a number from the vector, assuming run length coding */
int get_run_length(BITVEC *vp) {
    int ret;

    ret = get_delta(vp);
    if (ret > 0) {
        vp->last += ret;
        return (vp->last);
    }
    else
        return (-1);
}


/* Standard delta (Elias) */
int put_delta(BITVEC *vp, int n) {
    double floor(), log10();
    int mag;
    int ret;

    mag = (int) floor(L2((double) n));

    ret = put_gamma(vp, mag + 1);
    ret += put_n_bits_in_vector(vp, n, mag); /* don't output leftmost (ie, top) bit */

    return (ret);
}


/* Returns -1 on eof */
int get_delta(BITVEC *vp) {
    int mag, val;

    mag = get_gamma(vp) - 1;
    if (mag < 0)
        return (-1);
    val = get_n_bits_from_vector(vp, mag);
    /*printf("Noofbits = %d\n",mag);    cheong*/
    if (val < 0)
        return (-1);
#ifdef VECDEBUG
    if( ( ( 1 << mag ) | val ) < 0 )
	printf("Panic in getdelta: overflow\n");
#endif /* VECDEBUG */
    return ((1 << mag) | val);
}


/* Standard gamma (Elias) */
int put_gamma(BITVEC *vp, int n) {
    double floor(), log10();
    int mag;
    int ret;

    mag = (int) floor(L2((double) n));

    ret = put_n_bits_in_vector(vp, 0, mag);
    ret += put_n_bits_in_vector(vp, n, mag + 1);

    return (ret);
}


/* Returns -1 on eof */
int get_gamma(BITVEC *vp) {
    int b, mag, val;

    for (mag = 0; (b = get_bit(vp)) == 0; mag++);
    if (b < 0) {
#ifdef VECDEBUG
	if( vp->pos < vp->len/8 )
	    printf("Early termination in gamma (1).\n");
#endif /* VECDEBUG */
        return (-1);
    }
    val = get_n_bits_from_vector(vp, mag);
    if (val < 0) {
#ifdef VECDEBUG
	if( vp->pos < vp->len/8 )
	    printf("Early termination in gamma (1).\n");
#endif /* VECDEBUG */
        return (-1);
    }
#ifdef VECDEBUG
    if( ( ( 1 << mag ) | val ) < 0 )
	printf("Panic in getgamma: overflow\n");
#endif /* VECDEBUG */
    return ((1 << mag) | val);
}


/* Put n bits in vector, starting with nth from right, going to rightmost */
/* This is not efficient.  Should be recoded to insert several bits
   simultaneously ... but not really worth the effort ... */
int put_n_bits_in_vector(BITVEC *vp, int n, int num) {
    int i;
    int ret;

    for (ret = 0, i = num - 1; i >= 0; i--)
        ret += put_bit(vp, (n >> i) & 0x1);
    return (ret);
}


static int masks[9] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};


/* Get n bits from vector */
/* Rather than calling get_bit for each bit, shifts several bits at
   once if this would empty the current byte; only makes it one or
   two percent faster. */
int get_n_bits_from_vector(BITVEC *vp, int num) {
    int mask, shift, val, b;

    b = val = 0;

#if true
    for (shift = 8 - vp->bit; num >= shift; num -= shift, shift = 8 - vp->bit)
        /* copy out whole of cur */
    {
        if (8 * vp->pos + vp->bit >= vp->len)
            return (-1);
        mask = masks[shift];
        val = (val << shift) | (vp->cur & mask);
        vp->bit = 0;
        vp->pos++;
        vp->cur = vp->vector[vp->pos];
    }
#endif /* true */

    /* Get any remaining bits */
    for (; num > 0 && (b = get_bit(vp)) >= 0; num--)
        val = (val << 1) | (b & 0x1);
#ifdef VECDEBUG
    if( val < 0 )
	printf("Panic in getnbits: overflow\n");
    if( b < 0 && vp->pos < vp->len/8 )
	printf("Early termination in getnbits.\n");
#endif /* VECDEBUG */
    return ((b < 0) ? -1 : val);
}


/* Put a bit into vector; bytes are filled left to right */
/* Because of the way bytes are filled, vectors can't be accessed until
   terminate_vector_for_writing has been called. */
int put_bit(BITVEC *vp, int b) {
    bitcount++;

    vp->cur = (vp->cur << 1) | (b & 0x1);
    vp->bit++;
    if (vp->bit == 8) /* go to next byte */
    {
        if (vp->pos >= vp->size)
            expand_vector(vp);

        vp->vector[vp->pos] = vp->cur & 0xff;
        vp->cur = 0;
        vp->pos++;
        vp->bit = 0;
    }
    return (1);
}


/* Get a bit from a vector */
int get_bit(BITVEC *vp) {
    int b;

    if (8 * vp->pos + vp->bit >= vp->len)
        return (-1);

    b = (vp->cur >> (7 - vp->bit)) & 0x1;
    vp->bit++;
    if (vp->bit == 8) {
        vp->pos++;
        vp->bit = 0;
        vp->cur = vp->vector[vp->pos];
    }
    return (b);
}


/* Terminate a vector */
void terminate_vector_for_writing(BITVEC *vp) {
    if (vp->pos >= vp->size) /* this is a bit yucky */
        expand_vector(vp);
    vp->vector[vp->pos] = (vp->cur << (8 - vp->bit)) & 0xff;
    vp->len = 8 * vp->pos + vp->bit;
}

