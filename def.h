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


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#ifdef SOLARIS
#include <sys/times.h>
#include <limits.h>
#include <libgen.h>
#else /* SOLARIS */
#include <sys/time.h>
#include <sys/resource.h>
#endif /* SOLARIS */

#define	VOCAB		"vocab"
#define INDSUF		"index"
#define SKIPSUF 	"skip"	
#define PARAMSUF	"params"
#define MAXVOC		11000000		/* limit on vocab size */
#define	QUERYMAX	100000		/* limit on number of answers */
#define NUMANS		10		/* answers to ranked queries */
#define BUFLEN		100000
#define INITVECLEN	8		/* start size of index entry */
#define THRESHOLD	10
#define	BLOCKING	1
#define DISKBLOCK	13		/* log2 of assumed size of disk block */
#define MINSKIP 	4
#define true		1
#define false		0
#ifdef SLOWMATCH
#define SLOWDOWN	100		/* slow down factor for regex */
#endif /* SLOWMATCH */

/* Kind of soundex transformation used */
#define NOSOUNDEX	0
#define SOUNDEX		1
#define NTSOUNDEX	2
#define ASOUNDEX	3
#define PHONIX		4
#define NTPHONIX	5
#define PARTPHONIX	6
#define METAPHONE   7

/* Kind of fine search mechanism used */
#define PLAIN		0		/* ie., none */
#define QGRAM		1
#define PRIORITY	2		/* form of ranking */
#define EDIT3		3		/* edit3 */
#define EDIT4		4		/* edit4 */
/* #define EDITRAW	5		/ edit3 on orig string even if
					   phonetic transformation used */
#define EDITEX		6		/* edit with soundex help */
#define EDIT3TAPER	7		/* edit that tapers off */
#define EDITEXTAPER	8		/* edit with soundex help, tapered */
#define ONECHAR		9		/* allow up to one error */
#define LCS		10		/* longest common subsequence */

/* The DIRECT flag controls the kind of indexing -- normal or direct */
#define DIRECT		false

/* Short form of output */
#define BRIEF		true

#define MAXGRAMLEN	8
#ifndef GRAMLEN
#define GRAMLEN		2		/* Parameterised length of gram */
#endif /* GRAMLEN */

/* Structure to hold bitvector */
typedef struct bitvecrec
{
    char	*vector;	/* Sequence of bytes to hold bitvector */
    int		size;		/* Of vector, in bytes */
    int		pos;		/* Current byte number */
    int		bit;		/* Current bit in byte */
    int		cur;		/* Temporary space for putting, getting bits */ 
    int		len;		/* Number of bits used */
    int		last;		/* Total of runlengths seen so far */
		/* The last field is an oddity; it allows us to have
		   several bitvectors open at once */
} BITVEC;

/* Tree node for a gram and its vector */
typedef struct gramtreenode
{
    char	gram[GRAMLEN];	/* Characters of gram */
    BITVEC	vec;		/* Containing ptrs to words with gram */
    struct gramtreenode *left, *right, *par;
} TNODE;

/* Hash table node for a gram and its vector */
typedef struct gramhashnode
{
    char	gram[GRAMLEN];	/* Characters of gram */
    BITVEC	vec;		/* Containing ptrs to words with gram */
    struct gramhashnode *next;
} HNODE;

/* Table node for a gram and its vectors */
typedef struct gramnode
{
    char        gram[GRAMLEN];  /* Characters of gram */
    BITVEC      vec;            /* Containing ptrs to words with gram */
    BITVEC      svec;           /* Containing skip ptrs */
    BITVEC      saddrvec;       /* Containing ptrs to the ptrs in vec */
    struct gramnode *next;
} SNODE; 


#define L2(f)		( log10((f)) / 0.301029995 /*=log10(2.0)*/ )


void terminate_vector_for_writing(BITVEC *vp);
void write_vector_to_file(BITVEC *vp, FILE *to);
void initialise_vector(BITVEC *vp);
int read_vector_from_file(BITVEC *vp, FILE *fp);

