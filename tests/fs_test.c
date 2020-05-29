#include "../src/file_system.h"
#include "../src/registry.h"
#include "../src/utils.h"
#include <commons/string.h>
#include <stdio.h>
#include <stdbool.h>
#include <cspecs/cspec.h>
#include <commons/bitarray.h>

context (FS) {

    describe("FILE SYSTEM") {

        TableMetadata metadata = {"SC", 4, 60000};

        before {
            set_base_path("./LISSANDRA_FS_TEST/");
            create_db_blocks();
        } end
        
        after {
            create_db_blocks();
        } end

        describe("On init") { 

            it("should have all blocks created") {
                t_list* blocks = all_files_in(blocks_base_path());
                should_int(list_size(blocks)) be equal to(read_blocks_count());
            } end

            it("should have bitmap totally free") {
                t_bitarray* bitmap = read_bitmap();
                for (int i = 0; i < bitarray_get_max_bit(bitmap); i++) //TODO: Times
                {
                    should_bool(bitarray_test_bit(bitmap, i)) be falsey;
                }
            } end

            it("should get next block as first one") {
                should_int(next_block()) be equal to(0);
            } end

        } end

        describe("When write first block") { 

            before {
                write_block(0, "DATA");
            } end


            it("should have data inside") {
                should_string(read_block(0)) be equal to("DATA");
            } end

            it("should set bitmap position as used") {
                should_bool(bitarray_test_bit(read_bitmap(), 0)) be truthy;
            } end

            it("should get next block as second one") {
                should_int(next_block()) be equal to(1);
            } end

        } end

        describe("When write") { 

            it("small data should go to one block") {
                t_list* blocks = write_data("SMALL");
                should_int(list_size(blocks)) be equal to(1);
                list_destroy(blocks);
            } end

            it("limit data should go to one block") {
                t_list* blocks = write_data(string_repeat('A', read_block_size()));
                should_int(list_size(blocks)) be equal to(1);
                list_destroy(blocks);                
            } end

            it("big data should go to two blocks") {
                t_list* blocks = write_data(string_repeat('A', read_block_size() + 1));
                should_int(list_size(blocks)) be equal to(2);
                list_destroy(blocks);                
            } end

            it("big data should go to consecutive blocks") {
                t_list* blocks = write_data(string_repeat('A', read_block_size() + 1));
                should_int(list_get(blocks, 0)) be equal to(0);
                should_int(list_get(blocks, 1)) be equal to(1);
                list_destroy(blocks);                
            } end

            it("big data should split into blocks") {
                t_list* blocks = write_data(string_repeat('A', read_block_size() + 3));
                should_string(read_block(0)) be equal to(string_repeat('A', read_block_size()));
                should_string(read_block(1)) be equal to("AAA");
                list_destroy(blocks);                
            } end

        } end


        describe("When table is created") { 

            before {
                create_new_table("TABLE", metadata);
            } end
            
            after {
                delete_table("TABLE");
            } end

            it("should have partitions created") {
                times_do(4, (void*)
                    lambda(void, (int n), {
                        should_int(read_partition_size("TABLE", n)) be equal to(0);
                        should_int(array_length(read_partition_blocks("TABLE", 0))) be equal to(0);
                    })
                );
            } end

            it("should save data") {
                write_in_partition("TABLE", new_data(0, 0, "DATA"));
                should_int(read_partition_size("TABLE", 0)) be equal to(8);
                should_int(array_length(read_partition_blocks("TABLE", 0))) be equal to(1);
                should_string(read_block(0)) be equal to("0;0;DATA");
            } end

            it("should read data") {
                // write_in_partition
            } end

        } end

    } end
}