#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <getopt.h>

#ifdef EVILCHECK
#  include <math.h>
#endif

#ifndef VERSION
#  define VERSION "[devel]"
#endif

typedef int bool;
#define false 0
#define true 1

typedef signed char bit;
#define Q 0
#define O -1
#define X 1

#define max(p,q) ((p)>(q))?(p):(q)

typedef struct tPicture
{
  unsigned int counter; // how many Q-fields do we have
  bit bits[];
} tPicture;

tPicture *mainpicture;
unsigned int *leftborder, *topborder;
uint64_t *gtestfield;
unsigned int xsize, ysize, xysize, vsize;
unsigned int lmax, tmax;

uint64_t fingercounter;

bool optionColor = true; // can we use color tricks?
bool optionLinux = false; // can we make extended use of linux console?
bool optionHTML = false; // should we print HTML instead of plain text?
bool optionStats = false;

inline void message(char *message, ...)
{
  va_list ap;
  va_start(ap, message);
  if (!optionStats)
    vfprintf(stderr, message, ap);
  va_end(ap);
}

char freadchar(FILE *file)
// Synopsis:
// | reads one char from a file represented by `file' variable
// | if there's nothing to read or an error ocurred, returns '\0'
// | otherwise, returns the char
{
  char buf='\0';
  fread((void*)&buf, sizeof(char), 1, file);
  return buf;
}

char readchar(void)
// Synopsis: 
// | reads one char from standard input
// | if there's nothing to read or an error ocurred, returns '\0'
// | otherwise, returns the char
{
  char buf='\0';
  fread((void*)&buf, sizeof(char), 1, stdin);
  return buf;
}

void pf(const char *str)
// Synopsis:
// | prints a string `str' to standard output
{
  fwrite(str, strlen(str), 1, stdout);
}

void mpf(unsigned int count, ...)
// Synopsis:
// | prints strings to standard output
{
  unsigned int i;
  va_list ap;
  va_start(ap, count);
  for (i=0; i<count; i++)
    pf(va_arg(ap, char*));
  va_end(ap);
}

#ifdef EVILCHECK
double binomln(int n, int k)
// Synopsis:
// | evaluates ln binom(n,k)
// | if possible, returns the result
// | otherwise, returns +0.0
{
  double tmp;
  
  if (n<=k || n<=0 || k<=0)
    return 0.0;
  
  double dn = (double)n;
  double dk = (double)k;
  
  tmp=-0.5*log(8*atan(1)); // atan 1 = pi / 4
  tmp+=(dn+0.5)*log(dn);
  tmp-=(dk+0.5)*log(dk);
  tmp-=(dn-dk+0.5)*log(dn-dk);
  return tmp;
}
#endif

void nnSignal(int sn)
// Synopsis:
// | signal support function
// | says `Ouch!'
// | if necessary, turns all colors off
// | `sn' is a signal number which we have caught
{
  fprintf(stderr,"Ouch! (%d)\n\n",sn);
#ifdef LINUX
  if (optionColor) fprintf(stderr,"\x1B[0m");
#endif
  exit(EXIT_FAILURE);
}

void nnErrorOOM(void)
// Synopis:
// | tells about OOM error
// | afterwards, aborts
{
  fprintf(stderr,"Out of memory! ");
  abort();
}

void nnErrorInput(unsigned int n)
// Synopsis:
// | tells about invalid input at line `n'
// | afterwards, aborts
{
  fprintf(stderr,"Invalid input at line %u! ",n);
  abort();
}

void* xcalloc(size_t size)
// Synopsis:
// | allocates `size' bytes of memory
// | if necessary, handles errors
{
  void *tmp = calloc(1,size);
  if (!tmp)
    nnErrorOOM();
  return tmp;
}

void nnDrawPicturePlain(bit *picture, bit* cpicture)
// Synopsis:
// | vanilla version of generic picture printer
// | `picture' is a picture with our result
// | `cpicture' is a picture with which we want to comapre our result
{
  char *strLight, *strLight2, 
    *strDark, *strColor, *strError,
    *strH, *strV, *strTL, *strBL, *strTR, *strBR,
    *strHash;
#ifdef DEBUG
  if (cpicture==NULL)
    cpicture=picture;
#endif
  
  strHash="##";
  strLight=strLight2=strDark=strError="";
  strV="|"; strH="--"; strTL=strTR=strBL=strBR="+";

#ifdef LINUX
  if (optionColor && !optionHTML)
  {
    strLight="\x1B[1;37;45m";
    strLight2="\x1B[1;37;46m";
    strError="\x1B[1;37;41m";
    strDark="\x1B[0m";
  }
#endif

#ifdef LINUXEXT
  if (optionLinux && !optionHTML)
  {
    if (optionColor)
      strHash="\x1B[12m11\x1B[10m";
    else
      strHash="\x1B[12m[[\x1B[10m";
    strH="\x8A\x8A"; strV="\x85";
    strTL="\x86"; strBL="\x83";
    strTR="\x8C"; strBR="\x89";
  }
#endif
    
  unsigned int i,j;
  unsigned int t;
  
  for (i=0; i<tmax; i++)
  {
    for (j=0; j<=2*lmax; j++) pf(" ");
    for (j=0; j<xsize; j++)
    {
      strColor=(j&1)?strLight:strLight2;
      t=topborder[j*ysize+i];
      if (t!=0)
        pf(strColor), printf("%2u",t), pf(strDark);
      else
        mpf(3,strColor,"  ",strDark);
    }
    pf("\n");
  }
  
  for (i=0; i<2*lmax; i++) pf(" ");
  pf(strTL); for (i=0; i<xsize; i++) pf(strH); pf(strTR); pf("\n");  
  for (i=0; i<ysize; i++)
  {
    for (j=0; j<lmax; j++) 
    {
      strColor=(j&1)?strLight:strLight2;
      t=leftborder[i*xsize+j];
      if (t!=0)
        pf(strColor), printf("%2u",t), pf(strDark);
      else
        mpf(3,strColor,"  ",strDark);
    }
    pf(strV);
    for (j=0; j<xsize; j++)
    {
      strColor=(j&1)?strLight:strLight2;
      switch (*picture)
      {
        case Q:
          mpf(3,strColor,"<>",strDark);
          break;
        case O:
#ifdef DEBUG
          if (*cpicture==X)
            mpf(2,strError,"..");
          else
#endif
            mpf(2,strColor,"  ");
          pf(strDark);
          break;
        case X:
#ifdef DEBUG
          if (*cpicture==O)
            mpf(2,strError,strHash);
          else
#endif
            mpf(2,strColor,strHash);
          pf(strDark);
          break;
      }
      picture++;
#ifdef DEBUG
      cpicture++;
#endif
    }
    mpf(2,strV,"\n");
  }
  for (i=0; i<2*lmax; i++) pf(" ");
  pf(strBL); for (i=0; i<xsize; i++) pf(strH); mpf(2,strBR,"\n\n");
}

#ifdef CHROME
void nnDrawPictureHTML(bit *picture)
// Synopsis
// | HTML version of generic picture printer
// | `picture' is a picture with our result
{
  unsigned int i,j;
  unsigned int t;
  
  pf("<html>\n"
     "<head>\n"
     "<style type=\"text/css\">\n"  
     "  td, th {font: 8pt Arial, sans-serif; width: 11pt; height: 11pt;}\n"
     "  td.full  {background-color: #000000; color: white; border-left: solid 1px #808080; border-top: solid 1px #808080;}\n"
     "  td.empty {background-color: #F0F0F0; color: red; border-left: solid 1px #808080; border-top: solid 1px #808080;}\n"
     "</style>\n"
     "</head>\n"
     "<body>\n"
     "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">");
  
  for (i=0; i<tmax; i++)
  {
    pf("<tr>");
    for (j=0; j<lmax; j++) pf("<th></th>");
    for (j=0; j<xsize; j++)
    {
      t=topborder[j*ysize+i];
      if (t!=0)
        printf("<th>%u</th>",t);
      else
        pf("<th>&nbsp;</th>");
    }
    pf("</tr>\n");
  }
  
  for (i=0; i<ysize; i++)
  {
    pf("<tr>");
    for (j=0; j<lmax; j++) 
    {
      t=leftborder[i*xsize+j];
      if (t!=0)
        printf("<th>%u</th>",t);
      else
        pf("<th>&nbsp;</th>");
    }
    for (j=0; j<xsize; j++,picture++)
      switch (*picture)
      {
        case Q:
          pf("<td class=\"empty\">?</td>");
          break;
        case O:
          pf("<td class=\"empty\">&nbsp;</td>");
          break;
        case X:
          pf("<td class=\"full\">&nbsp;</td>");
          break;
      }
    pf("</tr>\n");
  }
  pf("</table>\n</body>\n</html>\n");
}
#else
void nnDrawPictureHTML(bit *picture)
{
  optionColor=false;
  pf("<html>\n<body>\n<pre>\n");
  nnDrawPicturePlain(picture,NULL);
  pf("</pre>\n</body>\n</html>\n");
}
#endif

inline void nnDrawPicture(bit *picture, bit* cpicture)
// Synopsis:
// | picture printer
// | automatically choses an appropriate version of
// |   generic printer
// | `picture' is a picture with our result
// | `cpicture' is a picture with which we want to comapre our result
{
  if (optionStats)
    return; // undocumented!
  if (optionHTML)
    nnDrawPictureHTML(picture);
  else
    nnDrawPicturePlain(picture,cpicture);
}

uint64_t nnTouchLine(bit *picture, unsigned int range, uint64_t* testfield, unsigned int *borderitem, bool vert) 
// it's rather ``touch of the mind'' :=)
{
  unsigned int i, j, k, count, sum, mul;
  uint64_t z, ink;
  bool ok;
  
  fingercounter++;
  
  sum=count=0;
  mul=vert?xsize:1;

  for(i=0; borderitem[i]>0; i++)
  {
    count++;
    sum+=borderitem[i];
  }
  if (sum+count<=range+1)
  {
    k=borderitem[0];
    z=0;
    if (count==1)
      for(i=0; i+k<=range; i++)
      {
        ok=true;
        for(j=0; j<i; j++)             if (picture[j*mul]==X) { ok=false; break; };
        if (!ok) break;
        for(j=i; j<i+k && ok; j++)     if (picture[j*mul]==O) ok=false;
        for(j=i+k; j<range && ok; j++) if (picture[j*mul]==X) ok=false;
        if (ok)
        {
          for(j=i; j<i+k; j++) testfield[j]++;
          z++;
        }
      }
    else
    {
      for(i=0; i<=range-sum-count+1; i++)
      {
        ok=true;
        for(j=0; j<i; j++)         if (picture[j*mul]==X) { ok=false; break; };
        if (!ok) break;
        for(j=i; j<i+k && ok; j++) if (picture[j*mul]==O) ok=false;
        if (!ok) continue; 
        if (i+k<range && picture[(i+k)*mul]==X) continue;
        j=i+k+1;
        ink=(count==1)?1:nnTouchLine(picture+j*mul,range-j,testfield+j,borderitem+1,vert);
        if (ink!=0)
        {
          for(j=i;j<i+k;j++)
            testfield[j]+=ink;
          z+=ink;
        }
      }
    }
    return z;
  }
  else 
    return 0;
}

inline bool nnQueueEmpty(unsigned int *queue)
// Synopsis:
// | checks if a queue (which `queue' points to) is empty
{
  return queue[0]==0;
}

bool nnQueuePush(unsigned int *queue, unsigned int i)
// Synopsis:
// | pushes a number (`i') to a queue (which `queue' points to)
// | if the number has been already queued, nothing happens
{
  i++;
  if (queue[i] == 0)
  {
    queue[i]=queue[0];
    if (queue[i] == 0)
      queue[i]=i;
    queue[0]=i;
    return true;
  }
  else
    return false;
}

int nnQueuePop(unsigned int *queue)
// Synopsis:
// | pops a number from a queue (which `queue' points to)
// | if the queue is empty, the result might be strange
{
  unsigned int r = queue[0];
  queue[0]=queue[r];
  if (queue[0] == r)
    queue[0]=0;
  queue[r]=0;
  return r-1;
}

void nnFingerLine(tPicture *mpicture, unsigned int* queue) // shouldn't it be splitted into two functions?
{
  bit *picture;
  uint64_t *testfield;
  uint64_t q, u;
  unsigned int i, c, imul, mul, size, line;
  bool vert;
  
  fingercounter++;
  line=nnQueuePop(queue);
  if (line<ysize)
    imul=xsize, mul=1, size=xsize, vert=false;
  else
    imul=1, mul=xsize, size=ysize, line-=ysize, vert=true;
  
  picture=mpicture->bits + line*imul;
  for (i=c=0; i<size; i++)
  {
    if (*picture == Q)
      c++;
    picture+=mul;
  }
  if (c==0 || c==size)
    return;
  
  picture=mpicture->bits + line*imul;
  testfield=gtestfield;
  memset(testfield, 0, size*sizeof(uint64_t));

  if (vert)
    q=nnTouchLine(picture, size, testfield, &topborder[line*size], true);
  else
    q=nnTouchLine(picture, size, testfield, &leftborder[line*size], false);

  for (i=0; i<size; i++)
  {
    u=*(testfield++);
    if ((u==q || u==0) && (*picture==Q))
    {
      mpicture->counter--;
      nnQueuePush(queue,i+(vert?0:ysize));
      *picture=u?X:O;
    }
    picture+=mul;
  }
}

bool nnIsConsistent(bit *picture)
{
  bool fr;
  unsigned int i, j;
  unsigned int r, rv;
  unsigned int *border;
  bit* tpicture = picture;
  
  for(i=0; i<ysize; i++)
  {
    fr=true;
    r=0;
    rv=0;
    border=leftborder+(i*xsize);
    for(j=0; j<xsize && fr; j++,tpicture++)
    {
      switch(*tpicture)
      {
        case Q: fr=false; break;
        case X: rv++; break;
        case O:
          if (rv>0)
          {
            if (*border!=rv) return false;
            rv=0; r++, border++;
          }
      }
      if (!fr) break;
    }
    if (fr && *border!=rv) return false;
  }

  for(i=0;i<xsize;i++)
  {
    fr=true;
    r=0;
    rv=0;
    border=topborder+(i*ysize);
    tpicture=picture+i;
    for(j=0; j<ysize && fr; j++,tpicture+=xsize)
    {
      switch(*tpicture)
      {
        case Q: fr=false; break;
        case X: rv++; break;
        case O:
          if (rv>0)
          {
            if (*border!=rv) return false;
            rv=0; r++, border++;
          }
      }
      if (!fr) break;
    }
    if (fr && *border!=rv) return false;
  }
  
  return true;
}

inline void* nnQAlloc(unsigned int size) 
// Synopsis:
// | allocates a queue of size `size'
{ 
  return xcalloc((size+1)*sizeof(unsigned int)); 
}

inline void nnQFree(void *queue) 
// Synopsis:
// | frees `queue'
{ 
  free(queue); 
}

inline void* nnBAlloc(void) 
// Synopsis:
// | allocates a band
{ 
  return xcalloc(vsize*sizeof(unsigned int)); 
}

inline void* nnTAlloc(void) 
// Synopsis
// | allocates a testfield
{ 
  return xcalloc(xysize*sizeof(uint64_t)); 
}

void* nnNAlloc(void) 
// Synopsis:
// | allocates a picture
{ 
  tPicture *tmp = xcalloc(sizeof(unsigned int) + vsize*sizeof(bit)); 
  tmp->counter=vsize;
  return tmp;
}

inline void nnNFree(void* picture)
// Synopsis:
// | frees `ptr'
{ 
  free(picture); 
}

inline void nnNCopy(tPicture *source, tPicture *destination) 
// Synopsis:
// | copies picture `source' onto picture `destination'
{ 
  memcpy(destination, source, sizeof(unsigned int)+vsize*sizeof(bit)); 
}

void nnFirstShake(tPicture *mpicture) 
// Note: 
// | namely, this is a QuickShake :=)
{
  unsigned int i, j, k;
  unsigned int R, ML;
  bit *picture;
  unsigned int *band;
  
  for (i=0;i<ysize;i++)
  {
    band=&leftborder[i*xsize];
    R=*(band++);
    ML=R;
    while (*band>0)
      ML += *(band++) + 1;

    band=&leftborder[i*xsize]; j=0;
    while (*band > 0)
    {
      k=xsize-ML;
      if (k < R)
      {
        picture=&mpicture->bits[i*xsize+k];
        for (; k<R; k++,picture++)
          if (*picture==Q)
          {
            *picture=X;
            mpicture->counter--;
          }
      }
      ML -= (*band); ML--;
      R++; R += *(++band);
    }  
  }

  for (i=0;i<xsize;i++)
  {
    band=&topborder[i*ysize];
    R=*(band++);
    ML=R;
    while (*band > 0)
      ML += *(band++) + 1;

    band=&topborder[i*ysize]; j=0;
    while (*band > 0)
    {
      k=ysize-ML;
      if (k < R)
      {
        picture=&mpicture->bits[k*xsize+i];
        for (; k<R; k++,picture+=xsize)
          if (*picture==Q)
          {
            *picture=X;
            mpicture->counter--;
          }
      }
      ML -= (*band); ML--;
      R++; R += *(++band);
    }  
  }  
}

inline void nnShake(tPicture *mpicture)
{
  unsigned int i;
  unsigned int *queue = nnQAlloc(xsize+ysize);

  for (i=0; i<ysize; i++)
    nnQueuePush(queue, i);

  for (i=0;i<xsize;i++)
    nnQueuePush(queue, ysize+i);
  
  while(!nnQueueEmpty(queue))
    nnFingerLine(mpicture, queue);

  nnQFree(queue);
  return;
}

bool nnUnambiguous(tPicture *mpicture)
{
  tPicture* mclone;
  bit *picture;
  unsigned int n;
  bool res = false;
  
  mclone=nnNAlloc();
  picture=mpicture->bits;
  for(n=0; n<vsize && !res; n++, picture++)
  {
    if (*picture!=Q)
      continue;
    assert(*picture==Q);
    nnNCopy(mpicture, mclone);
    mclone->bits[n]=O;
    nnShake(mclone);
    res=nnUnambiguous(mclone);
    if (res)
      nnNCopy(mclone, mpicture);
    else
    {
      *picture=X;
      nnShake(mpicture);
    }
  }
  nnNFree(mclone);
  return nnIsConsistent(mpicture->bits);;
}

void nnUsage(void)
// Synopsis:
// | prints out usage information and exits
// Note:
// | there is no magic here -- sorry
{
  fprintf(stderr,
    "Usage: nonogram [options]\n\n"
    "Options:\n"
#ifdef LINUX
    "  -m, --mono        don't use colors\n"
#endif
    "  -H, --html        HTML output\n"
#ifdef DEBUG
    "  -f, --file=FILE   validate the result using FILE\n"
#endif
    "  -h, --help        display this help and exit\n"
    "  -v, --version     output version information and exit\n\n");
  exit(EXIT_FAILURE);
}

void nnVersion(void)
// Synopsis:
// | prints out version information and exits
// Note:
// | there is no magic here -- sorry
{
  fprintf(stderr,"Nonogram v. " VERSION " -- a nonogram solver.\n"
                 "Copyright 2003/2004 Jakub Wilk <ubanus@users.sf.net>\n\n");
  exit(EXIT_FAILURE);
}

void nnParseArgs(int argc, char **argv, char** vfn)
// Synopsis:
// | parses the program arguments
// | `vfn' variable is used only if DEBUG directive is defined
// | otherwise, it's ignored
// | we are using getopt (!)
{
  static struct option options[]=
  {
    {"version",0,0,'v'},
    {"help",0,0,'h'},
    {"mono",0,0,'m'},
    {"html",0,0,'H'},
    {"file",0,0,'f'},
    {"statistics",0,0,'s'}, // undocumented
    {0,0,0,0}
  };

  int optindex, c;

  while(true)
  {
    optindex = 0;
    c=getopt_long(argc,argv,"vhmHsf:",options,&optindex);
    if (c<0)
      break;
    if (c==0)
      c=options[optindex].val;
    switch(c)
    {
      case 'v':
        nnVersion();
      case 'h':
        nnUsage();
      case 'm':
        optionColor=false;
        break;
      case 'H':
        optionHTML=true;
        break;
#ifdef DEBUG
      case 'f':
        if (optarg)
          *vfn=optarg;
        break;
#endif
      case 's':
        optionStats=true;
        break;
    }
  }
}


int main(int argc, char **argv)
{
  char c;
  unsigned int i, j, k, sane;
  bit* checkbits = NULL;

#ifdef DEBUG
  FILE* verifyfile;
  tPicture* checkpicture;
  char* verifyfname = NULL;
#endif

#ifdef EVILCHECK
  double evn1 = 0;
  double evn2 = 0;
  unsigned int evs, evm;
#endif

  signal(SIGINT, nnSignal);
  signal(SIGABRT, nnSignal);

#ifdef DEBUG
  nnParseArgs(argc, argv, &verifyfname);
#else
  nnParseArgs(argc, argv, NULL);
#endif

#ifdef LINUX
  if (!isatty(STDOUT_FILENO))  
    optionColor=false;
  else
  {
#ifdef LINUXEXT
    if (!strcmp(getenv("TERM"), "linux"))
      optionLinux=true;
#endif
  }
#endif

  xsize=ysize=0;
  c=readchar();
  while (c>='0' && c<='9') { xsize*=10; xsize+=c-'0'; c=readchar(); }
  while (c==' ' || c=='\t') c=readchar();
  while (c>='0' && c<='9') { ysize*=10; ysize+=c-'0'; c=readchar(); }
  while (c<=' ') c=readchar();
  
  if (xsize<1 || ysize<1 || xsize>999 || ysize>999)
    nnErrorInput(1);
  
  vsize=xsize*ysize;
  xysize=max(xsize,ysize);

  leftborder=nnBAlloc();
  topborder =nnBAlloc();
  gtestfield=nnTAlloc();

  mainpicture=nnNAlloc();

#ifdef EVILCHECK
  evs=evm=0;
#endif
  sane=-1;
  lmax=0;
  for (i=j=0;i<ysize;)
  {
    k=0;
    while (c>='0' && c<='9') { k*=10; k+=c-'0'; c=readchar(); }
    if (k==0) nnErrorInput(2+i);
    sane+=k+1;
    if (sane>xsize) nnErrorInput(2+i);
    leftborder[i*xsize+j]=k;
#ifdef EVILCHECK
    evs+=k;
    if (k>evm)
      evm=k;
#endif
    while (c==' ' || c=='\t') c=readchar();
    if (c=='\r' || c=='\n' || c=='\0')
    {
      i++;
      if (j>lmax)
        lmax=j;
#ifdef EVILCHECK
      evn1+=binomln(xsize-evs+1,j+1);
      evn2+=(double)(evs+evm+j)-(double)(1+xsize);
      evs=evm=0;
#endif        
      j=0;
      sane=-1;
      c=readchar();
    }
    else
      j++;
  }

  assert(sane == -1);
  tmax=0;
  for (i=j=0;i<xsize;)
  {
    k=0;
    while (c>='0' && c<='9') { k*=10; k+=c-'0'; c=readchar(); }
    if (k==0) nnErrorInput(2+ysize+i);
    sane+=k+1;
    if (sane>ysize) nnErrorInput(2+ysize+i);
    topborder[i*ysize+j]=k;
#ifdef EVILCHECK
    evs+=k;
    if (k>evm)
      evm=k;    
#endif
    while (c==' ' || c=='\t') c=readchar();
    if (c=='\r' || c=='\n' || c=='\0')
    {
      i++;
      if (j>tmax)
        tmax=j;
#ifdef EVILCHECK
      evn1+=binomln(ysize-evs+1,j+1);
      evn2+=(double)(evs+evm+j)-(double)(1+ysize);
      evs=evm=0;
#endif        
      j=0;
      sane=-1;
      c=readchar();
    }
    else
      j++;
  }

  lmax++;
  tmax++;

#ifdef EVILCHECK
  evn1/=(double)(xsize+ysize);
  evn2=exp(-evn2/(double)vsize);
  message("Evilness measure: %.2fe\n",pow(evn2,evn1));
#endif

#ifdef DEBUG
  if (verifyfname!=NULL)
  {
    verifyfile=fopen(verifyfname,"r");
    if (verifyfile!=NULL)
    {
      checkpicture=nnNAlloc();
      checkpicture->counter=0;
      c=0;
      for(i=0;i<ysize;i++)
      {
        while(c<' ') 
          c=freadchar(verifyfile);
        for(j=0;j<xsize;j++)
        {
          checkpicture->bits[i*xsize+j]=(c=='#')?X:O;
          freadchar(verifyfile);
          c=freadchar(verifyfile);
        }
      }
      fclose(verifyfile);
      checkbits=checkpicture->bits;
    }
  }
#endif

  fingercounter=0;
  nnFirstShake(mainpicture);
  nnShake(mainpicture);

  if (!nnIsConsistent(mainpicture->bits))
  {
    fingercounter=0;
    message("Inconsistent! Bamf!\n");
#ifdef DEBUG
    nnDrawPicture(mainpicture->bits, checkbits);
#endif
  }
  else if (mainpicture->counter==0)
    nnDrawPicture(mainpicture->bits, checkbits);
  else
  {
    message("Ambiguous! But trying to find a solution...\n");
    if (nnUnambiguous(mainpicture))
      nnDrawPicture(mainpicture->bits, checkbits);
    else
    {
      fingercounter=0;
      message("Not only ambiguous but also inconsistent! Bamf!\n");
    }
  }
  
  if (optionStats)
    fprintf(stderr, "%ju\n", fingercounter);

  return EXIT_SUCCESS;
}
