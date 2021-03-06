typedef union {  char *str;
          int number;
       } YYSTYPE;
#define	STR	258
#define	GSTR	259
#define	VAR	260
#define	NUMBER	261
#define	WINDOWTITLE	262
#define	WINDOWSIZE	263
#define	WINDOWPOSITION	264
#define	FONT	265
#define	FORECOLOR	266
#define	BACKCOLOR	267
#define	SHADCOLOR	268
#define	LICOLOR	269
#define	OBJECT	270
#define	INIT	271
#define	PERIODICTASK	272
#define	MAIN	273
#define	END	274
#define	PROP	275
#define	TYPE	276
#define	SIZE	277
#define	POSITION	278
#define	VALUE	279
#define	VALUEMIN	280
#define	VALUEMAX	281
#define	TITLE	282
#define	SWALLOWEXEC	283
#define	ICON	284
#define	FLAGS	285
#define	WARP	286
#define	WRITETOFILE	287
#define	HIDDEN	288
#define	CANBESELECTED	289
#define	NORELIEFSTRING	290
#define	CASE	291
#define	SINGLECLIC	292
#define	DOUBLECLIC	293
#define	BEG	294
#define	POINT	295
#define	EXEC	296
#define	HIDE	297
#define	SHOW	298
#define	CHFORECOLOR	299
#define	CHBACKCOLOR	300
#define	GETVALUE	301
#define	CHVALUE	302
#define	CHVALUEMAX	303
#define	CHVALUEMIN	304
#define	ADD	305
#define	DIV	306
#define	MULT	307
#define	GETTITLE	308
#define	GETOUTPUT	309
#define	GETASOPTION	310
#define	SETASOPTION	311
#define	STRCOPY	312
#define	NUMTOHEX	313
#define	HEXTONUM	314
#define	QUIT	315
#define	LAUNCHSCRIPT	316
#define	GETSCRIPTFATHER	317
#define	SENDTOSCRIPT	318
#define	RECEIVFROMSCRIPT	319
#define	GET	320
#define	SET	321
#define	SENDSIGN	322
#define	REMAINDEROFDIV	323
#define	GETTIME	324
#define	GETSCRIPTARG	325
#define	IF	326
#define	THEN	327
#define	ELSE	328
#define	FOR	329
#define	TO	330
#define	DO	331
#define	WHILE	332
#define	BEGF	333
#define	ENDF	334
#define	EQUAL	335
#define	INFEQ	336
#define	SUPEQ	337
#define	INF	338
#define	SUP	339
#define	DIFF	340


extern YYSTYPE yylval;
