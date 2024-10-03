#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <cstdarg>
using namespace std;

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

typedef struct mp_index {
    int bank;  // 0:9
    int field; // A:0, B:1
    int mp;    // 0:23
} MP_INDEX;

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

bool read_raw_data(const char* filename, uint8_t* raw_dat, int num_bank, int num_row, int num_col)
{
    ifstream fs(filename, ios::binary);
    if(!fs) {
        cerr << "cannot open " << filename << endl;
        return false;
    }
    int j, i, k;
    for (j = 0; j < num_bank; ++j) {
        for (i = 0; i < num_row; ++i) {
            for (k = 0; k < num_col; ++k) {
                    fs >> raw_dat[j][i][k];
            }
        }
    }
    
    fs.close();
    return true;
}