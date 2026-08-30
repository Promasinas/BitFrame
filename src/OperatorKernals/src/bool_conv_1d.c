#include "bool_conv_1d.h"

bool bool_conv_1d_forward(size_t input_block_index,
                          size_t kernel_block_index,
                          size_t output_block_index,
                          size_t stride,
                          size_t padding,
                          size_t dilation,
                          bool   is_loop){
    memory_block_t input_block = get_block_by_index(input_block_index);
    memory_block_t kernel_block = get_block_by_index(kernel_block_index);
    memory_block_t output_block = get_block_by_index(output_block_index);

    if(input_block.addr == NULL || kernel_block.addr == NULL || output_block.addr == NULL){
        return false;
    }

    return bool_conv_1d_forward_by_addr(input_block.addr,
                                        input_block.block_size,
                                        kernel_block.addr,
                                        kernel_block.block_size,
                                        output_block.addr,
                                        output_block.block_size,
                                        stride,
                                        padding,    
                                        dilation,
                                        is_loop);
}



