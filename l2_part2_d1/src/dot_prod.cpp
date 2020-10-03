#include "dot_prod.hpp"

//TODO (1): Set block size
#define BLOCK_SIZE ??


//TRIPCOUNT identifier
const unsigned int v_len = BLOCK_SIZE;

extern "C" {
void dot_prod(const int *vec_a,     // Read-Only Vector 1
          const int *vec_b,         // Read-Only Vector 2
          int *results,             // Output Result
          const unsigned int vector_len, //Vector size
) {

//TODO (2): Determine the correct depth
#pragma HLS INTERFACE m_axi port = vec_a offset = slave depth=?? bundle = gmem
#pragma HLS INTERFACE m_axi port = vec_b offset = slave depth=?? bundle = gmem
#pragma HLS INTERFACE m_axi port = results offset = slave depth=?? bundle = gmem

#pragma HLS INTERFACE s_axilite port = vec_a bundle = control
#pragma HLS INTERFACE s_axilite port = vec_b bundle = control
#pragma HLS INTERFACE s_axilite port = results bundle = control
#pragma HLS INTERFACE s_axilite port = vector_len bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

    unsigned int va_buffer[BLOCK_SIZE];   // Local SRAM to store vector A
    unsigned int vb_buffer[BLOCK_SIZE];   // Local SRAM to store vector B
    unsigned int vout_buffer[1];          // Local Register to store result

    load:
        //TODO (3): Load the buffers
        for(int i = 0; i < BLOCK_SIZE; i++){
            //????
        }

    compute:
        //TODO (4) do the dot product!
        for(int i = 0; i < BLOCK_SIZE; i++){
            //????
        }

    store:
        //TODO (5): write vout_buffer back to results
        results[??] = vout_buffer[??];
    
}
