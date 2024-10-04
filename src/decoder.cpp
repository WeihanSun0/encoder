#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;

#include "common.h"

int main()
{

    cout << "decoder v0.0.1" << endl;
   
    bool res;
    //read raw data
    res = read_raw_data(outfile_raw_dat, (uint8_t *)raw_dat, num_bank, num_lines_per_bank, bytes_row_per_bank);
    if(!res)
        exit(1);
    //read ebd
    res = read_decoded_dat(outfile_ebd, (uint8_t *)EBD, ebd_h, ebd_w);
    if(!res)
        exit(1);
    //read histogram
    res = read_decoded_dat(outfile_hist, (uint32_t*)histogram, hist_h, hist_h);
    if(!res)
        exit(1);
    //read ref
    res = read_decoded_dat(outfile_ref, (uint32_t*)Ref, ref_h, ref_h);
    if(!res)
        exit(1);
    //read pdc 
    res = read_decoded_dat(outfile_pdc, (uint32_t*)PDC, pdc_h, pdc_h);
    if(!res)
        exit(1);
    exit(0);
}
