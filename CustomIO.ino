/* 
* File: Custom_IO
* Origin: 20-Oct-2018
* Author: Tip Partridge
* Descrition:
*		Generic output subroutines. Intended to be customized for specific application.
*   Used by EEPROMDump.
*/
// *******************************************************************************

// *****************************************************************
// IO Subroutines
// *****************************************************************

void outstr( char *instr)
//
  {
  Serial.print( instr);
  }

// *****************************************************************

void outchr( char inchr)
//
  {
  Serial.print( inchr);
  }

// *****************************************************************

