#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <cstdarg>
using namespace std;
#define DUMMY_DAT 77 //FULLBANK
#define DUMMY_DAT2 0 //FULLSCAN

const char outfile_raw_dat_10b[] = "raw_dat_10bank.bin";
const char outfile_raw_dat_43b[] = "raw_dat_43bank.bin";
const char outfile_ebd[] = "ebd_dat.csv";
const char outfile_pdc[] = "pdc_dat.csv";
const char outfile_hist[] = "hist_dat.csv";
const char outfile_ref[] = "ref_dat.csv";
const char outfile_mp_table[] = "mp_table.csv";
const char outfile_intensity[] = "intentsity.csv";

const size_t num_bank = 10; // FULLBANK mode
const size_t num_bank2 = 43; // FULLSCAN mode
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
const size_t bytes_ref_per_bank = bytes_valid_per_bank;
const size_t num_uint32_valid_per_bank = bytes_valid_per_bank / 4;
const size_t bytes_row_per_bank = bytes_ebd_per_bank + bytes_invalid_per_bank;
const size_t num_lines_per_bank = num_mp * num_field + num_ref + num_ebd; // edb line
// output size
const size_t hist_h = num_dp, hist_w = num_uint32_valid_per_bank;
const size_t ebd_h = num_bank, ebd_w = bytes_ebd_per_bank;
const size_t ref_h = num_ref * num_bank, ref_w = num_uint32_valid_per_bank;
const size_t pdc_h = num_dp, pdc_w = num_valid_pdc;
const size_t raw_h = num_bank * num_lines_per_bank;
const size_t intensity_h = 140, intensity_w = 170;

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
// intensity
uint32_t intensity[intensity_h][intensity_w] = {};

// MP index 
typedef struct mp_index {
    int bank;  // 0:9
    int field; // A:0, B:1
    int mp;    // 0:23
} MP_INDEX;

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
uint8_t raw_dat2[num_bank2][num_lines_per_bank][bytes_row_per_bank] = {};
/// @brief print data to stream, delimiter=,
/// @tparam T datatype
/// @param dat 2d array data
/// @param h row num
/// @param w col num
/// @param stream output stream
template<typename T>
void print_dat2stream(T* dat, const size_t& h, const size_t& w, ostream& stream)
{
    for (size_t j = 0; j < h; ++j) {
        for (size_t i = 0; i < w; ++i) {
            stream << *(dat + j*w + i) << ",";
        }
        stream << endl;
    }
}


/// @brief print mp index to stream for test
/// @param dat mp_index 
/// @param h height
/// @param w width
/// @param stream output stream
void print_mp_index(MP_INDEX* dat, int h, int w, ostream& stream)
{
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            MP_INDEX p = *(dat + j * w + i);
            stream << p.bank << "-" << (p.field == 0? "A":"B") << "-" << setw(2) << setfill('0') << p.mp << ",";
        }
        stream << endl;
    }
}

/// @brief fill dummy data
/// @tparam T typename 
/// @param dat dat in/out
/// @param h height
/// @param w width
/// @param fix_value fixed value
/// @param random false: use fixed value
template<typename T>
void fill_dummy_dat(T* dat, int h, int w, int fix_value = 1, bool random=false) 
{
    int max_value = 10;
    if (random) {
        srand(time(0));
    }
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            *(dat + j * w + i) = random ? (rand() % max_value + 1) : fix_value;
        }
    }
}

/// @brief read raw data from file
/// @param filename file name 
/// @param dat output dat
/// @param num_bank number of banks
/// @param num_row number of rows per bank
/// @param num_col number of cols per bank
/// @return 
bool read_raw_data(const char* filename, uint8_t* dat, int num_bank, int num_row, int num_col)
{
    ifstream fs(filename, ios::binary);
    fs.seekg(0, std::ios::end);
    std::streampos fileSize = fs.tellg();
    fs.seekg(0, std::ios::beg);
    cout << "file size = " << fileSize << endl;
    if(!fs) {
        cerr << "cannot open " << filename << endl;
        return false;
    }
    int j, i, k;
    for (j = 0; j < num_bank; ++j) {
        int bytes_per_bank = num_row * num_col;
        for (i = 0; i < num_row; ++i) {
            for (k = 0; k < num_col; ++k) {
                fs >> *(dat + j * bytes_per_bank + i * num_col + k);
            }
        }
    }
    
    fs.close();
    return true;
}

template<typename T>
bool read_decoded_dat(const char* filename, T* dat, const size_t& h, const size_t& w)
{
    ifstream fs(filename);
    if (!fs) {
        cerr << "cannot open " << filename << endl;
        return false;
    }
    for (size_t j = 0; j < h; ++j) {
        for (size_t i = 0; i < w; ++i) {
            fs >> *(dat + j*w + i);
        }
    }
    fs.close();
    return true;
}