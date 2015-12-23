/*
 * adc.h
 *
 * Created: 20.01.2013 15:14:36
 *  Author: Jörg
 */ 


#ifndef ADC_H_
#define ADC_H_

#include "integer.h"

// Voltage to ADC value 
#define TO_ADC( x )   (((WORD)(x * 36.0 + 0.5)))

class Adc
{
public:
  
  static void init();
  
  static void cyclic(BYTE tickerFlag);
  
  /**
   *  one Adc bit is about 1/36 Volt, so calculate the voltage as value/36
   */
  static WORD getValue()          { return value; }
  static WORD getLimit()          { return limit; }
  static void setLimit(WORD w)    { limit= w; }
  
  static BYTE isLimitReached();
    
  // return number of LiPo Cells
  static BYTE numberOfCells();
  
private:
  
  static WORD value;
  static WORD limit;
};



#endif /* ADC_H_ */