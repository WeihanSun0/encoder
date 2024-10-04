#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;
#include "common.h"


#define OUTPUT_FILE
// #define OUTPUT
// #define TEST_PRINT




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

/// @brief create mp index table for FULLBANK, value is BANK-FIELD-MP
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

/// @brief create raw data of FULLSCAN. after set dummy dat of intensity 
void create_raw_dat_fullscan()
{
    int i, j, m, n;
    MP_INDEX mp;
    int bank_index = 0;
    int mp_index = 0;
    uint8_t *des;
    // uint32_t pixel_per_block[6][2] = {};
    uint32_t pixels_in_block[12] = {};
    memset(raw_dat2, DUMMY_DAT2, num_bank2 * num_lines_per_bank * bytes_row_per_bank * sizeof(uint8_t));
    // field A 
    for (i = 0; i < intensity_h/2; i+=2) { // height
        mp.field = 0;
        for (j = intensity_w-1; j >= 0; j-=6) { // widith
            mp.bank = bank_index;
            mp.mp = mp_index;
            // 12 4-bytes to 48 bytes in a block
            for (m = 0; m < 6; m++) { // cols
                for (n = 0; n < 2; n++) { // rows
                    pixels_in_block[(5-m)*2+n] = intensity[i + n][(j - 6) + m];
                }
            }
            des = &raw_dat2[mp.bank][num_ebd + num_ref + mp.mp * num_field + mp.field][bytes_extra_per_bank + bytes_valid_per_bank];
            memcpy(des, pixels_in_block, 48);
            // index increase
            bank_index += 1;
            if (bank_index >= 42) {//bank index
                bank_index = 0;
                mp_index += 1;
            }
        }
    }
    // field 0
    bank_index = 0, mp_index = 0;
    for (i = intensity_h/2; i < intensity_h; i+=2) { // height
        mp.field = 1;
        for (j = intensity_w-1; j >= 0; j-=6) { // widith
            mp.bank = bank_index;
            mp.mp = mp_index;
            // 12 4-bytes to 48 bytes in a block
            for (m = 0; m < 6; m++) { // cols
                for (n = 0; n < 2; n++) { // rows
                    pixels_in_block[(5-m)*2+n] = intensity[i + n][(j - 6) + m];
                }
            }
            des = &raw_dat2[mp.bank][num_ebd + num_ref + mp.mp * num_field + mp.field][bytes_extra_per_bank + bytes_valid_per_bank];
            memcpy(des, pixels_in_block, 48);
            // index increase
            bank_index += 1;
            if (bank_index >= 42) {//bank index
                bank_index = 0;
                mp_index += 1;
            }
        }
    }
}

/// @brief create raw data of FULLBANK. after set lut and dummy dat of ref, ebd, pdc and histogram
void create_raw_dat_fullbank()
{
    int i, j;
    uint8_t *des;
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
        des = raw_dat[j][0];
        memcpy(des, EBD[j], bytes_ebd_per_bank);
    }
    // copy ref
    for (j = 0; j < num_bank; ++j) {
        for (i = 0; i < num_ref; ++i) {
            des = &raw_dat[j][num_ebd+i][bytes_extra_per_bank];
            memcpy(des, Ref[j*2+i], bytes_ref_per_bank);
        }
    }
    // copy PDC
    for (j = 0; j < num_bank; ++j) {
        for (i = 0; i < num_field * num_mp; ++i) {
            if (LUT_MP2DP[j][i] >= num_dp) {//invalid
                continue;
            }
            des = &raw_dat[j][num_ebd+i][bytes_extra_per_bank+bytes_valid_per_bank];
            memcpy(des, PDC[LUT_MP2DP[j][i]], num_valid_pdc*sizeof(uint32_t));
        }
    }
}


/// @brief save files
/// @return 
bool output_files()
{
    int i, j, k;
    // raw data of FULLBANK
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

    // intensity
    ofstream fs_intensity(outfile_intensity);
    if (fs_intensity.is_open()) {
        print_dat2stream((uint32_t*)intensity, intensity_h, intensity_w, fs_intensity);
    } else {
        return false;
    }
    fs_intensity.close();
    // raw data of FULLSCAN
    ofstream fs2(outfile_raw_dat, ios::binary);
    if (fs2.is_open()) {
        for (j = 0; j < num_bank; ++j) {
            for (i = 0; i < num_lines_per_bank; ++i) {
                for (k = 0; k < bytes_row_per_bank; ++k) {
                    fs2 << raw_dat2[j][i][k];
                }
            }
        }
    } else {
        return false;
    }
    fs2.close();
    return true;
}


int main()
{
    cout << "encoder v0.0.1" << endl;
    // create gt
    fill_dummy_dat<uint32_t>((uint32_t *)histogram, hist_h, hist_w, 2, false);
    fill_dummy_dat<uint8_t>((uint8_t *)EBD, ebd_h, ebd_w, 3, false);
    fill_dummy_dat<uint32_t>((uint32_t *)Ref, ref_h, ref_w, 4, false);
    fill_dummy_dat<uint32_t>((uint32_t *)PDC, pdc_h, pdc_w, 5, false);
    fill_dummy_dat<uint32_t>((uint32_t *)intensity, intensity_h, intensity_w, 7, false);
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
    create_raw_dat_fullbank();
    create_raw_dat_fullscan();

#ifdef OUTPUT_FILE
    cout << "output files" << endl;
    output_files();
#endif
    return 0;
}