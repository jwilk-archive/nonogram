#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#if 1
#include <stdbool.h>
#else
  typedef int bool;
# define true 1
# define false 0
#endif

#ifdef DEBUG
#  define debug 1
#else
#  define debug 0
#endif

#ifndef VERSION
#  define VERSION "[devel]"
#endif

typedef signed char bit;
#define Q 0
#define O (-1)
#define X 1

#define max(p,q) ((p)>(q))?(p):(q)

#define c_maxsize 999
#define c_maxfactor 10000
#define c_maxevilness 15.0

#ifdef __TINYC__
#  define add64(a, b) a=a+b
#else
#  define add64(a, b) a+=b
#endif

typedef struct
{
  unsigned int counter; // how many Q-fields do we have
  unsigned int *linecounter;
  unsigned int *evilcounter;
  bit bits[];
} tPicture;

typedef struct
{
  unsigned int id;
  int factor;
} tQueueElement;

typedef struct
{
  unsigned int size;
  unsigned int *enqueued;
  tQueueElement *elements;
  char space[];
} tQueue;

tPicture *mainpicture;
unsigned int *leftborder, *topborder;
uint64_t *gtestfield;
unsigned int xsize, ysize, xysize, xpysize, vsize;
unsigned int lmax, tmax;

uint64_t fingercounter;

bool optionColor = true; // can we use color tricks?
bool optionLinux = false; // can we make extended use of linux console?
bool optionHTML = false; // shall we print HTML instead of plain text?
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
  fputs(str, stdout);
}

void mpf(unsigned int count, ...)
// Synopsis:
// | prints `count' strings to standard output
{
  unsigned int i;
  va_list ap;
  va_start(ap, count);
  for (i=0; i<count; i++)
    pf(va_arg(ap, char*));
  va_end(ap);
}

double binomln(int n, int k)
// Synopsis:
// | evaluates ln binom(`n', `k')
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

void nnSignal(int sn)
// Synopsis:
// | signal support function
// | says ``Ouch!''
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
  void *tmp = calloc(1, size);
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
  if (debug && cpicture==NULL) 
    cpicture=picture;
  
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
      if (t!=0 || i==0)
        pf(strColor), printf("%2u", t), pf(strDark);
      else
        mpf(3, strColor, "  ", strDark);
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
      if (t!=0 || j==0)
        pf(strColor), printf("%2u", t), pf(strDark);
      else
        mpf(3, strColor, "  ", strDark);
    }
    pf(strV);
    for (j=0; j<xsize; j++)
    {
      strColor=(j&1)?strLight:strLight2;
      switch (*picture)
      {
        case Q:
          mpf(3, strColor, "<>", strDark);
          break;
        case O:
          if (debug && *cpicture == X)
            mpf(2, strError, "..");
          else
            mpf(2, strColor, "  ");
          pf(strDark);
          break;
        case X:
          if (debug && *cpicture == O)
            mpf(2, strError, strHash);
          else
            mpf(2, strColor, strHash);
          pf(strDark);
          break;
      }
      picture++;
      debug && cpicture++;
    }
    mpf(2, strV, "\n");
  }
  for (i=0; i<2*lmax; i++) pf(" ");
  pf(strBL); for (i=0; i<xsize; i++) pf(strH); mpf(2, strBR, "\n\n");
  fflush(stdout);
}

#ifdef CHROME
void nnDrawPictureHTML(bit *picture)
// Synopsis
// | HTML version of generic picture printer
// | `picture' is a picture with our result
{
  unsigned int i, j, t;
  
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
      if (t != 0)
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

  for (i=0; borderitem[i]>0; i++)
  {
    count++;
    sum+=borderitem[i];
  }
  if (sum+count <= range+1)
  {
    k=borderitem[0];
    z=0;
    if (count==1)
      for (i=0; i+k<=range; i++)
      {
        ok=true;
        for (j=0; j<i; j++)             if (picture[j*mul]==X) { ok=false; break; };
        if (!ok) break;
        for (j=i; j<i+k && ok; j++)     if (picture[j*mul]==O) ok=false;
        for (j=i+k; j<range && ok; j++) if (picture[j*mul]==X) ok=false;
        if (ok)
        {
          for (j=i; j<i+k; j++) add64(testfield[j], 1);
          add64(z, 1);
        }
      }
    else
    {
      for (i=0; i<=range-sum-count+1; i++)
      {
        ok=true;
        for (j=0; j<i; j++)         if (picture[j*mul]==X) { ok=false; break; };
        if (!ok) break;
        for (j=i; j<i+k && ok; j++) if (picture[j*mul]==O) ok=false;
        if (!ok) continue; 
        if (i+k<range && picture[(i+k)*mul]==X) continue;
        j=i+k+1;
        ink=(count==1)?1:nnTouchLine(picture+j*mul, range-j, testfield+j, borderitem+1, vert);
        if (ink != 0)
        {
          for (j=i; j<i+k; j++)
            add64(testfield[j], ink);
          add64(z, ink);
        }
      }
    }
    return z;
  }
  else 
    return 0;
}

inline tQueue* nnQueueAlloc(void) 
// Synopsis:
// | allocates a queue
{ 
  tQueue *tmp = 
    xcalloc(
      (int)((char*)(tmp->space) - (char*)tmp) +
      xpysize*(sizeof(unsigned int*) + sizeof(tQueueElement)) );
  tmp->size=0;
  tmp->enqueued=(unsigned int*)tmp->space;
  memset(tmp->enqueued, -1, sizeof(unsigned int*)*xpysize);
  tmp->elements=(tQueueElement*)(tmp->space+xpysize*sizeof(unsigned int));
  return tmp;
}

inline void nnQueueFree(void *queue) 
// Synopsis:
// | frees `queue'
{ 
  free(queue);
}

inline bool nnQueueEmpty(tQueue *queue)
// Synopsis:
// | checks if a queue (which `queue' points to) is empty
{
  return queue->size==0;
}

inline void nnQueueUpdateEnq(tQueue *queue, unsigned int i)
{
  assert(i < queue->size);
  queue->enqueued[queue->elements[i].id]=i;
}

void nnQueueHeapify(tQueue *queue)
{
  unsigned int i, l, r, max;
  tQueueElement ivalue;
  
  i=0;
  while (true)
  {
    ivalue=queue->elements[i];
    l=2*i+1;
    r=l+1;
    if (l<queue->size && queue->elements[l].factor>ivalue.factor)
      max=l;
    else
      max=i;
    if (r<queue->size && queue->elements[r].factor>queue->elements[max].factor)
      max=r;
    if (max!=i)
    {
      queue->elements[i]=queue->elements[max];
      queue->elements[max]=ivalue;
      nnQueueUpdateEnq(queue, i);
      nnQueueUpdateEnq(queue, max);
      i=max;
    }
    else 
      return;
  }
}

bool nnQueuePush(tQueue *queue, unsigned int id, int factor)
// Synopsis:
// | pushes a number (`i') to a queue (which `queue' points to)
// | if the number has been already queued, the queue might be renumbered
{
  unsigned int i, j;
  
  factor=-factor;

  assert(id < xpysize);
  i=queue->enqueued[id];
  if (i == (unsigned int)-1)
    i=queue->size++;
  else
  {
    assert(i < queue->size);
    if (factor <= queue->elements[i].factor)
      return false;
  }
  
  while(i>0 && queue->elements[j=(i-1)/2].factor < factor)
  {
    queue->elements[i]=queue->elements[j];
    nnQueueUpdateEnq(queue, i);
    i=j;
  }
  
  queue->elements[i].id=id;
  queue->elements[i].factor=factor;
  nnQueueUpdateEnq(queue, i);

  return true;
}

int nnQueuePop(tQueue *queue)
// Synopsis:
// | pops a number from a queue (which `queue' points to)
// | if the queue is empty, the result might be strange
{
  unsigned int resultid, last;
  assert(queue->size > 0);
  resultid=queue->elements[0].id;
  last=--queue->size;
  if (queue->size > 0)
  {
    queue->elements[0]=queue->elements[last];
    nnQueueUpdateEnq(queue, 0);
    nnQueueHeapify(queue);
  }
  queue->enqueued[resultid]=(unsigned int)-1;
  return resultid;
}

void nnFingerLine(tPicture *mpicture, tQueue* queue) 
// shouldn't it be splitted into two functions?
{
  bit *picture;
  uint64_t *testfield;
  uint64_t q, u;
  unsigned int i, j, imul, mul, size, oline, line;
  int factor;
  bool vert;
    
  fingercounter++;
  factor=queue->elements[0].factor;
  line=oline=nnQueuePop(queue);
  if (line < ysize)
    imul=xsize, mul=1, size=xsize, vert=false;
  else
    imul=1, mul=xsize, size=ysize, line-=ysize, vert=true;

  j=mpicture->linecounter[oline];  
  if (j==0 || j==size)
    return;
  
  picture=mpicture->bits + line*imul;
  testfield=gtestfield;
  memset(testfield, 0, size*sizeof(uint64_t));

  if (vert)
    q=nnTouchLine(picture, size, testfield, &topborder[line*size], true);
  else
    q=nnTouchLine(picture, size, testfield, &leftborder[line*size], false);

  j=vert?0:ysize;
  for (i=j; i<j+size; i++)
  {
    u=*(testfield++);
    if ((u==q || u==0) && (*picture==Q))
    {
      mpicture->counter--;
      mpicture->linecounter[oline]--;
      factor=c_maxfactor*(--mpicture->linecounter[i])/size + mpicture->evilcounter[i];
      nnQueuePush(queue, i, factor);
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
  bit* tpicture;
  
  for(i=0; i<ysize; i++)
  {
    fr=true;
    r=0;
    rv=0;
    border=leftborder+(i*xsize);
    tpicture=picture+xsize*i;
    for (j=0; j<xsize && fr; j++,tpicture++)
    {
      switch (*tpicture)
      {
        case Q: fr=false; break;
        case X: rv++; break;
        case O:
          if (rv>0)
          {
            if (*border!=rv) 
            {
              debug && fprintf(stderr, "Inconsistency at row #%u[%u]! (%u, expected %u)!\n", i, j, rv, *border);
              return false;
            }
            rv=0; r++; border++;
          }
      }
    }
    if (fr && *border!=rv)
    {
      debug && fprintf(stderr, "Inconsistency at the end of row #%u! (%u, expected %u)\n", i, rv, *border);
      return false;
    }
  }

  for (i=0;i<xsize;i++)
  {
    fr=true;
    r=0;
    rv=0;
    border=topborder+(i*ysize);
    tpicture=picture+i;
    for (j=0; j<ysize && fr; j++,tpicture+=xsize)
    {
      switch (*tpicture)
      {
        case Q: fr=false; break;
        case X: rv++; break;
        case O:
          if (rv>0)
          {
            if (*border!=rv) 
            {
              debug && fprintf(stderr, "Inconsistency at column #%u[%u]! (%u, expected %u)\n", i, j, rv, *border);
              return false;
            }
            rv=0; r++; border++;
          }
      }
    }
    if (fr && *border!=rv) 
    {
      debug && fprintf(stderr, "Inconsistency at the end of column #%u! (%u, expected %u)\n", i, rv, *border);
      return false;
    }
  }
  
  return true;
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
  unsigned int i;
  tPicture *tmp = 
    xcalloc(
      (int)((char*)(tmp->bits) - (char*)tmp) +
      vsize*sizeof(bit) );
  tmp->linecounter=xcalloc(sizeof(unsigned int)*xpysize);
  tmp->evilcounter=xcalloc(sizeof(unsigned int)*xpysize);
  for (i=0; i<ysize; i++)
    tmp->linecounter[i]=xsize;
  for (i=0; i<xsize; i++)
    tmp->linecounter[ysize+i]=ysize;
  tmp->counter=vsize;
  return tmp;
}

inline void nnNFree(tPicture *picture)
// Synopsis:
// | frees `picture'
{ 
  free(picture->linecounter);
  free(picture->evilcounter);
  free(picture);
}

inline void nnNCopy(tPicture *src, tPicture *dst) 
// Synopsis:
// | copies picture `src' onto picture `dst'
{ 
  dst->counter=src->counter;
  memcpy(dst->linecounter, src->linecounter, sizeof(unsigned int)*xpysize);
  memcpy(dst->evilcounter, src->evilcounter, sizeof(unsigned int)*xpysize);
  memcpy(dst->bits, src->bits, vsize*sizeof(bit)); 
}

void nnFirstShake(tPicture *mpicture) 
// Note: 
// | actually, this is a QuickShake :=)
{
  unsigned int i, j, k;
  unsigned int R, ML;
  bit *picture;
  unsigned int *band;
  
  for (i=0; i<ysize; i++)
  {
    band=&leftborder[i*xsize];
    R=*(band++);
    ML=R;
    while (*band>0)
      ML += *(band++) + 1;

    band=&leftborder[i*xsize];
    if (*band == 0)
    {
      picture=&mpicture->bits[i*xsize];
      for(j=0; j<xsize; j++, picture++)
      {
        *picture=O;
        mpicture->counter--;
      }
    }
    while (*band > 0)
    {
      k=xsize-ML;
      if (k < R)
      {
        picture=&mpicture->bits[i*xsize+k];
        for (; k<R; k++, picture++)
        {
          *picture=X;
          mpicture->counter--;
        }
      }
      ML -= (*band); ML--;
      R++; R += *(++band);
    }
  }

  for (i=0; i<xsize; i++)
  {
    band=&topborder[i*ysize];
    R=*(band++);
    ML=R;
    while (*band > 0)
      ML += *(band++) + 1;

    band=&topborder[i*ysize];
    if (*band == 0)
    {
      picture=&mpicture->bits[i];
      for(j=0; j<ysize; j++, picture+=xsize)
      if (*picture == Q)
      {
        *picture=O;
        mpicture->counter--;
      }
    }
    while (*band > 0)
    {
      k=ysize-ML;
      if (k < R)
      {
        picture=&mpicture->bits[k*xsize+i];
        for (; k<R; k++, picture+=xsize)
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

  picture=mpicture->bits;
  for (i=0; i<ysize; i++)
  for (j=0; j<xsize; j++, picture++)
  if (*picture != Q)
  {
    mpicture->linecounter[i]--;
    mpicture->linecounter[ysize+j]--;
  }
}

inline void nnShake(tPicture *mpicture)
{
  unsigned int i, j;
  int factor;
  tQueue* queue = nnQueueAlloc();
  
  for (i=0; i<ysize; i++)
  {
    factor=c_maxfactor*mpicture->linecounter[i]/xsize + mpicture->evilcounter[i];
    nnQueuePush(queue, i, factor);
  }

  for (i=0, j=ysize; i<xsize; i++, j++)
  {
    factor=c_maxfactor*mpicture->linecounter[j]/ysize + mpicture->evilcounter[i];
    nnQueuePush(queue, j, factor);
  }

  while (!nnQueueEmpty(queue))
    nnFingerLine(mpicture, queue);

  nnQueueFree(queue);
  return;
}

bool nnUnambiguous(tPicture *mpicture)
{
  tPicture* mclone;
  bit *picture;
  unsigned int i, j, n;
  bool res = false;

  mclone=nnNAlloc();
  picture=mpicture->bits;
  n=0;
  for (i=0; i<ysize && !res; i++)
  for (j=0; j<xsize && !res; j++, n++, picture++)
  if (*picture==Q)
  {
    nnNCopy(mpicture, mclone); // mpicture --> mclone
    mclone->bits[n]=O;
    mclone->counter--;
    mclone->linecounter[i]--;
    mclone->linecounter[ysize+j]--;
    nnShake(mclone);
    res=nnUnambiguous(mclone);
    if (res)
      nnNCopy(mclone, mpicture); // mclone --> mpicture
    else
    {
      *picture=X;
      mpicture->counter--;
      mpicture->linecounter[i]--;
      mpicture->linecounter[ysize+j]--;
      nnShake(mpicture);
    }
  }
  nnNFree(mclone);
  return nnIsConsistent(mpicture->bits);
}

unsigned int nnMeasureEvilness(int r, int k)
{
  double tmp = binomln(r, k);
  if (tmp>c_maxevilness)
    tmp=c_maxevilness;
  return floor((tmp*c_maxevilness)*c_maxfactor);
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
    {"version", 0, 0,'v'},
    {"help", 0, 0, 'h'},
    {"mono", 0, 0, 'm'},
    {"html", 0, 0, 'H'},
    {"file", 0, 0, 'f'},
    {"statistics", 0, 0,'s'}, // undocumented
    {0, 0, 0, 0}
  };

  int optindex, c;
  
  while (true)
  {
    optindex = 0;
    c=getopt_long(argc,argv,"vhmHsf:",options,&optindex);
    if (c<0)
      break;
    if (c==0)
      c=options[optindex].val;
    switch (c)
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
      case 'f':
        if (debug && optarg)
          *vfn=optarg;
        break;
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
  unsigned int evs, evm;
  bit* checkbits = NULL;

#ifdef DEBUG
  FILE* verifyfile;
  tPicture* checkpicture;
#endif
  static char* verifyfname = NULL;

  signal(SIGINT, nnSignal);
  signal(SIGABRT, nnSignal);

  nnParseArgs(argc, argv, &verifyfname);

#ifdef LINUX
  if (!isatty(STDOUT_FILENO))  
    optionColor=false;
#ifdef LINUXEXT
  else
  {
    if (!strcmp(getenv("TERM"), "linux"))
      optionLinux=true;
  }
#endif
#endif

  xsize=ysize=0;
  c=readchar();
  while (c>='0' && c<='9') { xsize*=10; xsize+=c-'0'; c=readchar(); }
  while (c==' ' || c=='\t') c=readchar();
  while (c>='0' && c<='9') { ysize*=10; ysize+=c-'0'; c=readchar(); }
  while (c<=' ') c=readchar();
  
  if (xsize<1 || ysize<1 || xsize>c_maxsize || ysize>c_maxsize)
    nnErrorInput(1);
  
  vsize=xsize*ysize;
  xpysize=xsize+ysize;
  xysize=max(xsize,ysize);

  leftborder=nnBAlloc();
  topborder =nnBAlloc();
  gtestfield=nnTAlloc();

  mainpicture=nnNAlloc();

  evs=evm=0;
  sane=(unsigned int)-1;
  lmax=0;
  for (i=j=0; i<ysize; )
  {
    k=0;
    while (c>='0' && c<='9') { k*=10; k+=c-'0'; c=readchar(); }
    sane+=k+1;
    if ((sane>xsize) || (k==0 && j>0)) 
      nnErrorInput(2+i);
    leftborder[i*xsize+j]=k;
    evs+=k;
    if (k>evm)
      evm=k;
    while (c==' ' || c=='\t') c=readchar();
    if (c=='\r' || c=='\n' || c=='\0')
    {
      if (j>lmax)
        lmax=j;
      mainpicture->evilcounter[i]=nnMeasureEvilness(xsize-evs+1, j+1);
      evs=evm=0;
      i++;
      j=0;
      sane=-1;
      do
        c=readchar();
      while (c=='\r' || c=='\n');
    }
    else
      j++;
  }

  assert(sane == (unsigned int)-1);
  tmax=0;
  for (i=j=0; i<xsize; )
  {
    k=0;
    while (c>='0' && c<='9') { k*=10; k+=c-'0'; c=readchar(); }
    sane+=k+1;
    if ((sane>ysize) || (k==0 && j>0)) 
      nnErrorInput(2+ysize+i);
    topborder[i*ysize+j]=k;
    evs+=k;
    if (k>evm)
      evm=k;    
    while (c==' ' || c=='\t') c=readchar();
    if (c=='\r' || c=='\n' || c=='\0')
    {
      if (j>tmax)
        tmax=j;
      mainpicture->evilcounter[ysize+i]=nnMeasureEvilness(ysize-evs+1, j+1);
      evs=evm=0;
      i++;
      j=0;
      sane=-1;
      do
        c=readchar();
      while (c=='\r' || c=='\n');
    }
    else
      j++;
  }

  lmax++;
  tmax++;

#ifdef DEBUG
  if (verifyfname!=NULL)
  {
    verifyfile=fopen(verifyfname, "r");
    if (verifyfile!=NULL)
    {
      checkpicture=nnNAlloc();
      checkpicture->counter=0;
      c=0;
      for (i=0;i<ysize;i++)
      {
        while (c<' ') 
          c=freadchar(verifyfile);
        for (j=0;j<xsize;j++)
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
    if (debug)
      nnDrawPicture(mainpicture->bits, checkbits);
  }
  else
  {
    if ((mainpicture->counter==0) || debug)
      nnDrawPicture(mainpicture->bits, checkbits);
    if (mainpicture->counter!=0)
    {
      message("Ambiguous (%u)! But trying to find a solution...\n", mainpicture->counter);
      if (nnUnambiguous(mainpicture))
        nnDrawPicture(mainpicture->bits, checkbits);
      else
      {
        fingercounter=0;
        message("Inconsistent! Bamf!\n");
      }
    }
  }
  
  if (optionStats)
    fprintf(stderr, "%ju\n", fingercounter);

  return EXIT_SUCCESS;
}

/* vim:set ts=2 sw=2 et: */
