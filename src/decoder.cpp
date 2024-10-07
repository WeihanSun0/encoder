#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;
#include <opencv2/opencv.hpp>
using namespace cv;

#include "common.h"

void bank_integration(uint8_t* src, Mat& capture_raw_, int num)
{
    capture_raw_.create(num * num_lines_per_bank, bytes_row_per_bank, CV_8U);
    uint8_t* p_dst = reinterpret_cast<uint8_t*>(capture_raw_.ptr(0, 0));
    memcpy(src, p_dst, num * num_lines_per_bank * bytes_row_per_bank);

}

int main()
{
    cout << "decoder v0.0.1" << endl;
    bool res;
    //read raw data FULLBANK
    res = read_raw_data(outfile_raw_dat_10b, (uint8_t *)raw_dat, num_bank, num_lines_per_bank, bytes_row_per_bank);
    if(!res)
        exit(1);
    cv::Mat capture_raw_10b, capture_raw_43b;
    bank_integration((uint8_t*)raw_dat, capture_raw_10b, num_bank);
    //read raw data FULLSCAN
    res = read_raw_data(outfile_raw_dat_43b, (uint8_t *)raw_dat2, num_bank2, num_lines_per_bank, bytes_row_per_bank);
    bank_integration((uint8_t*)raw_dat2, capture_raw_43b, num_bank);
    cout << (uint32_t)capture_raw_43b.at<uint8_t>(9, 0) << endl;
    if(!res)
        exit(1);
    cv::Mat capture_raw;
    // read ebd
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
