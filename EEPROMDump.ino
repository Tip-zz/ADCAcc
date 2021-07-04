
// *****************************************************************

void EEPROMDump()
  {
  int EEsize;
  int rr, ii;
  char ASCII[17];
  char astr[81];
  char astr1[5];
  int nrow;
  int residue;
  byte abyte;

  EEsize = EEPROM.length();
  nrow = EEsize / 16;
  residue = EEsize - nrow * 16;
  outstr("\r\n");
  for (rr=0; rr<nrow; rr++)
    {
    hexstr(astr, rr*16, 4, false);
    catstr(astr, ": ");
    for (ii=0; ii<8; ii++)
      {
      abyte = EEPROM.read( rr*16+ii);
      hexstr(astr1, abyte, 2, false);
      catstr(astr, astr1);
      catstr(astr, " ");
      if ((abyte<32) || (abyte>126)) abyte = 46;
      ASCII[ii] = char(abyte);
      }
     catstr(astr, "- ");
    for (ii=8; ii<16; ii++)
      {
      abyte = EEPROM.read( rr*16+ii);
      hexstr(astr1, abyte, 2, false);
      catstr(astr, astr1);
      catstr(astr, " ");
      if ((abyte<32) || (abyte>126)) abyte = 46;
      ASCII[ii] = char(abyte);
      }
    ASCII[16] ='\0';
    catstr(astr, "  ");
    catstr(astr, ASCII);
    catstr(astr, "\r\n");
    outstr(astr);
    }
  if (residue > 0)
    {
    rr = nrow;
    hexstr( astr, rr*16, 4, false);
    catstr(astr, ": ");
    for (ii=0; ii<8; ii++)
      {
      if (ii<residue)
        {
        abyte = EEPROM.read( rr*16+ii);
        hexstr(astr1, abyte, 2, false);
        catstr(astr, astr1);
        catstr(astr, " ");
        if ((abyte<32) || (abyte>126)) abyte = 46;
        ASCII[ii] = char(abyte);
        }
      else
        {
        catstr(astr, "   ");
        ASCII[ii] = char(' ');
        }
      }
     catstr(astr, "- ");
    for (ii=8; ii<16; ii++)
      {
      if (ii<residue)
        {
        abyte = EEPROM.read( rr*16+ii);
        hexstr(astr1, abyte, 2, false);
        catstr(astr, astr1);
        catstr(astr, " ");
        if ((abyte<32) || (abyte>126)) abyte = 46;
        ASCII[ii] = char(abyte);
        }
      else
        {
        catstr(astr, "   ");
        ASCII[ii] = char(' ');
        }
      }
    ASCII[16] ='\0';
    catstr(astr, "  ");
    catstr(astr, ASCII);
    catstr(astr, "\r\n");
    outstr( astr);
    }
  }

// *****************************************************************


