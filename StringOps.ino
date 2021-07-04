/* 
* File: StringOps.ino
* Origin: 20-Oct-2018
* Author: Tip Partridge
* Descrition:
*		String related subroutines.
*/
// *******************************************************************************

// *****************************************************************
// Lower Level Subroutine
// *****************************************************************

void hexstr( char *rtnstr, unsigned int val, int ndig, bool prefix)
// format number val as hex, pad with 0 to ndig wide, prepend 0x, results into rtnstr, null terminated string.
{
  char s1[20];
  char s2[20];
  int ii, jj;
  unsigned int siz;
  int pads;
  
  if (ndig > 4) ndig = 4; // 16 bits only
//convert to hex
  sprintf( s1, "%x", val);
//pad as needed
  siz = len( s1);
  pads = ndig - siz;
  if (pads<0) pads = 0;
  for (ii=0; ii<pads; ii++)
    s2[ii] = '0';
//copy digits
  jj = siz - ndig + pads;
  while (ii < ndig)
    s2[ii++] = s1[jj++];
//null terminate
  s2[ii] = 0;
//add prefix if needed
  if (prefix)
    {
    sprintf(rtnstr, "0x%s", s2);
    rtnstr[ndig+2]=0;
    }
  else
    {
    sprintf(rtnstr, "%s", s2);
    rtnstr[ndig]=0;
    }
}

// *****************************************************************

void padstr( char *rtnstr, char *instr, unsigned int width, int align, char padchr)
// Size, align, and pad instr into rtnstr, null terminated string. 0=left, 1=center, 2=right
  {
  unsigned int siz;
  unsigned int i = 0;
  unsigned int j;
  unsigned int lef = 0;
  unsigned int rit = 0;

  siz = len( instr);
  if (siz > width) siz = width;
  switch (align)
    {
    case 0: {
      rit = width - siz;
      break;
      }
    case 1: {
      lef = width - siz;
      rit = lef / 2;
      lef = lef - rit;
      break;
      }
    default: {
      lef = width - siz;
      break;
      }
    }
  while ( i<lef)
    rtnstr[i++] = padchr;
  for (j=0; j<siz; j++)
    rtnstr[i++] = instr[j];
  for (j=0; j<rit; j++)
    rtnstr[i++] = padchr;
  rtnstr[i] = 0;
  }

// *****************************************************************

unsigned int len( char *strg)
// Return length of null terminated string
  {
  unsigned int n;

  n = 0;
  while (strg[n] != 0)
    n += 1;
  return n;
  }

// *****************************************************************

void catstr( char *rtnstr, char *instr)
// Concatinate rtnstr with instrSize, null terminated strings.t
  {
  int i;
  int j = 0;
  i = len( rtnstr);
  while (instr[j] != 0)
    rtnstr[i++] = instr[j++];
  rtnstr[i] = 0;
  }

// *****************************************************************

void quotestr( char *rtnstr, char *instr)
// Put quotes around instr and put into rtnstr, null terminated strings.
  {
  int i = 0;
  rtnstr[0] = '"';
  while (instr[i] != 0)
    rtnstr[i+1] = instr[i++];
  rtnstr[++i] = '"';
  rtnstr[++i] = 0;
  }

// *****************************************************************


