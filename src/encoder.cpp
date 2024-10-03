#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;
#include "common.h"

#define DUMMY_DAT 255 

#define OUTPUT_FILE
// #define OUTPUT
// #define TEST_PRINT
const char outfile_raw_dat[] = "raw_dat.bin";
const char outfile_ebd[] = "ebd_dat.csv";
const char outfile_pdc[] = "pdc_dat.csv";
const char outfile_hist[] = "hist_dat.csv";
const char outfile_ref[] = "ref_dat.csv";
const char outfile_mp_table[] = "mp_table.csv";

const size_t num_bank = 10;
const size_t num_dp = 360;
const size_t num_mp = 24;
const size_t num_field = 2;
const size_t num_ref = 2;
const size_t num_ebd = 1;
const size_t num_valid_pdc = 10;
// bank info
const size_t bytes_ebd_per_bank = 820;
const size_t bytes_invalid_per_bank = 12;
const size_t bytes_extra_per_bank = 4;
const size_t bytes_pdc_per_bank = 48;
const size_t bytes_valid_per_bank = 768;
const size_t num_uint32_valid_per_bank = bytes_valid_per_bank / 4;
const size_t bytes_row_per_bank = bytes_ebd_per_bank + bytes_invalid_per_bank;
const size_t num_lines_per_bank = num_mp * num_field + num_ref + num_ebd; // edb line
// output size
const size_t hist_h = num_dp, hist_w = num_uint32_valid_per_bank;
const size_t ebd_h = num_bank, ebd_w = bytes_ebd_per_bank;
const size_t ref_h = num_ref * num_bank, ref_w = num_uint32_valid_per_bank;
const size_t pdc_h = num_dp, pdc_w = num_valid_pdc;
const size_t raw_h = num_bank * num_lines_per_bank;

// reverse DP order
uint32_t histogram[hist_h][hist_w] = {};
// mp table (mid dat)
uint32_t mp_table[num_bank][num_field * num_mp][num_uint32_valid_per_bank] = {};
// BANK order
uint8_t EBD[ebd_h][ebd_w] = {};
// BANK order
uint32_t Ref[ref_h][ref_w] = {};
// DP order
uint32_t PDC[pdc_h][pdc_w] = {};


//table setting
const int dp_table_width = 40;
const int dp_table_height = 9;
const int mp_table_width = 48;
const int mp_table_height = 10;
//LUT
int LUT_MP2DP[mp_table_height][mp_table_width] = {};
MP_INDEX LUT_MP2BANK[mp_table_height][mp_table_width] = {};
// raw data (output)
uint8_t raw_dat[num_bank][num_lines_per_bank][bytes_row_per_bank] = {};

/// @brief create lut size of mp table, value is corresponding dp index
/// @param lut output table
void make_lut_mp2dp(int* lut)
{
    int start_x = 4, start_y = 0;
    int index = 360;
    memset(lut, index, mp_table_height * mp_table_width * sizeof(int));
    for (int j = 0; j < dp_table_height; ++j) {
        for (int i = 0; i < dp_table_width; ++i) {
            *(lut + (j + start_y)*mp_table_width + (i + start_x)) = --index;
        }
    }
}

/// @brief create mp index table, value is BANK-FIELD-MP
/// @param lut 
void make_lut_mp2bank(MP_INDEX* lut)
{
    for (int j = 0; j < mp_table_height; ++j) {
        for (int i = 0; i < mp_table_width; ++i) {
            MP_INDEX &p = *(lut + j * mp_table_width + i);
            // bank
            int base = j / 2 * 2;
            if (i % 2 == 0) {
                p.bank = base + 1;
            } else {
                p.bank = base;
            }
            // field
            if (((i+j*2)/2)%2 == 1) {
                p.field = 0;
            } else {
                p.field = 1;
            }
            // mp
            if((i/2)%2 == 0) {
                p.mp = 23 - i / 4;
            }else {
                p.mp = 11 - i / 4;
            }
        }
    }
}

/// @brief create raw data. after set lut and dummy dat of ref, ebd, pdc and histogram
void create_raw_dat()
{
    int i, j;
    uint8_t *des;
    // make mp_table
    memset(raw_dat, DUMMY_DAT, num_bank * num_lines_per_bank * bytes_row_per_bank * sizeof(uint8_t));
    // copy histogram
    for (j = 0; j < num_bank; ++j) {
        for (i = 0; i < num_field * num_mp; ++i) {
            if (LUT_MP2DP[j][i] >= num_dp) {//invalid
                continue;
            }
            MP_INDEX p = LUT_MP2BANK[j][i];
            des = &raw_dat[p.bank][num_ebd + num_ref + p.mp * num_field + p.field][bytes_extra_per_bank];
            memcpy(des, histogram[num_dp - 1 - LUT_MP2DP[j][i]], num_uint32_valid_per_bank * sizeof(uint32_t));
        }
    }
    // copy ebd
    for (j = 0; j < num_bank; ++j) {
        des = raw_dat[num_bank][0];
        memcpy(des, EBD, bytes_ebd_per_bank);
    }
    // copy ref
    for (j = 0; j < num_bank; ++j) {
        for (i = 0; i < num_ref; ++i) {
            des = &raw_dat[num_bank][num_ebd+i][bytes_extra_per_bank];
            memcpy(des, Ref[j*2+i], bytes_ebd_per_bank);
        }
    }
    // copy PDC
    for (j = 0; j < num_bank; ++j) {
        for (i = 0; i < num_field * num_mp; ++i) {
            if (LUT_MP2DP[j][i] >= num_dp) {//invalid
                continue;
            }
            des = &raw_dat[num_bank][num_ebd+i][bytes_extra_per_bank+bytes_valid_per_bank];
            memcpy(des, PDC[LUT_MP2DP[j][i]], num_valid_pdc*sizeof(uint32_t));
        }
    }
}


/// @brief save files
/// @return 
bool output_files()
{
    int i, j, k;
    // raw data
    ofstream fs(outfile_raw_dat, ios::binary);
    if (fs.is_open()) {
        for (j = 0; j < num_bank; ++j) {
            for (i = 0; i < num_lines_per_bank; ++i) {
                for (k = 0; k < bytes_row_per_bank; ++k) {
                    fs << raw_dat[j][i][k];
                }
            }
        }
    } else {
        return false;
    }
    fs.close();
    // ebd data
    ofstream fs_ebd(outfile_ebd);
    if (fs_ebd.is_open()) {
        print_dat2stream((uint8_t*)EBD, ebd_h, ebd_w, fs_ebd);
    } else {
        return false;
    }
    fs_ebd.close();
    
    //hist data
    ofstream fs_hist(outfile_hist);
    if (fs_hist.is_open()) {
        print_dat2stream((int*)histogram, hist_h, hist_w, fs_hist);
    } else {
        return false;
    }
    fs_hist.close();
    // pdc
    ofstream fs_pdc(outfile_pdc);
    if (fs_pdc.is_open()) {
        print_dat2stream((uint32_t*)PDC, pdc_h, pdc_w, fs_pdc);
    } else {
        return false;
    }
    fs_pdc.close();
    // ref
    ofstream fs_ref(outfile_ref);
    if (fs_ref.is_open()) {
        print_dat2stream((uint32_t*)Ref, ref_h, ref_w, fs_ref);
    } else {
        return false;
    }
    fs_ref.close();
    // print mp table -- debug
    ofstream fs_mp_table(outfile_mp_table);
    if (fs_mp_table.is_open()) {
        print_mp_index((MP_INDEX *)LUT_MP2BANK, mp_table_height, mp_table_width, fs_mp_table);
    } else {
        return false;
    }
    fs_mp_table.close();
    return true;
}


int main()
{
    cout << "encoder v0.0.1" << endl;
    cout << outfile_raw_dat << endl;
    // create gt
    fill_dummy_dat<uint32_t>((uint32_t *)histogram, hist_h, hist_w, 2, false);
    fill_dummy_dat<uint8_t>((uint8_t *)EBD, ebd_h, ebd_w, 3, false);
    fill_dummy_dat<uint32_t>((uint32_t *)Ref, ref_h, ref_w, 4, false);
    fill_dummy_dat<uint32_t>((uint32_t *)PDC, pdc_h, pdc_w, 5, false);
#ifdef OUTPUT 
    print_dat2stream((uint32_t*)histogram, hist_h, hist_w, cout);
    print_dat2stream((uint8_t*)EBD, ebd_h, ebd_w, cout);
    print_dat2stream((uint32_t*)Ref, ref_h, ref_w, cout);
    print_dat2stream((uint32_t*)PDC, pdc_h, pdc_w, cout);
#endif
    // make mp2dp lut
    make_lut_mp2dp((int*)LUT_MP2DP);
    // make mp_index table
    make_lut_mp2bank((MP_INDEX *)LUT_MP2BANK);
#ifdef OUTPUT
    print_dat2stream((int*)LUT_MP2DP, 10, 48, cout);
    print_mp_index((MP_INDEX *)LUT_MP2BANK, mp_table_height, mp_table_width, cout);
#endif
    // create raw data
    create_raw_dat();
    
#ifdef OUTPUT_FILE
#if 0
    cout << "output file" << endl;
    fstream fs(outfile_hist, ios::out);
    print_dat2stream((int*)histogram, hist_h, hist_w, fs);
    fs.close();
#endif
    output_files();
#endif
    return 0;
}