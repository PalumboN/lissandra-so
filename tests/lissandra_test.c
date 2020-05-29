#include <stdio.h>
#include <stdbool.h>
#include <cspecs/cspec.h>
#include "../src/file_system.h"
#include "../src/lissandra.h"
#include "../src/registry.h"
#include "../src/memtable.h"

context (LISSANDRA) {

    describe("LISSANDRA API") {

        Memtable* memtable;
        TableMetadata metadata = {"SC", 4, 60000};
        
        before {
            set_base_path("./LISSANDRA_FS_TEST/");
            memtable = memtable_create();
            lissandra_create("TABLE", metadata);
        } end

        after {
            // lissandra_drop("TABLE");
        } end

        describe("SELECT") {
            
            it("Older value") {//TODO: Validar que sea entre las 3 opciones
                lissandra_insert(memtable, "TABLE", new_data(0, 1, "HOLA"));
                lissandra_insert(memtable, "TABLE", new_data(1, 1, "CHAU"));
                should_string(lissandra_select(memtable, "TABLE", 1)) be equal to("CHAU");
            } end

            skip("From partitions") {
                should_string(lissandra_select(memtable, "TABLE", 1)) be equal to("CHAU");
            } end

            it("From memtable") {
                lissandra_insert(memtable, "TABLE", new_data(0, 1, "HOLA"));
                should_string(lissandra_select(memtable, "TABLE", 1)) be equal to("HOLA");
            } end

            it("From temp") {
                lissandra_insert(memtable, "TABLE", new_data(0, 1, "HOLA"));
                memtable_dump(memtable);
                should_string(lissandra_select(memtable, "TABLE", 1)) be equal to("HOLA");
            } end

            it("Table doesn't exist") {
                should_string(lissandra_select(memtable, "NOT_EXIST_TABLE", 1)) be equal to("Table doesn't exist: NOT_EXIST_TABLE");
            } end

            it("Key doesn't exist") {
                should_ptr(lissandra_select(memtable, "TABLE", 999)) be null;
            } end

        } end

        describe("INSERT") {

            it("Old key") {
                should_string(lissandra_insert(memtable, "TABLE", new_data(0, 1, "TRUE"))) be equal to("OK"); 
            } end

            it("New key") {
                should_string(lissandra_insert(memtable, "TABLE", new_data(0, 2, "TRUE"))) be equal to("OK"); 
            } end

            it("Table doesn't exist") {
                should_string(lissandra_insert(memtable, "NOT_EXIST_TABLE", new_data(0, 1, "TRUE"))) be equal to("Table doesn't exist: NOT_EXIST_TABLE");
            } end

        } end

        describe("CREATE") {

            after {
                lissandra_drop("NEWTABLE");
            } end

            it("New table") {
                should_string(lissandra_create("NEWTABLE", metadata)) be equal to("OK"); 
            } end

            it("Table already exist") {
                should_string(lissandra_create("TABLE", metadata)) be equal to("Table already exist: TABLE"); 
            } end

        } end


        describe("DROP") {

            after {
                lissandra_create("TABLE", metadata);
            } end
            
            it("TABLE") {
                should_string(lissandra_drop("TABLE")) be equal to("OK"); 
            } end

            it("Table doesn't exist") {
                should_string(lissandra_drop("NOT_EXIST_TABLE")) be equal to("Table doesn't exist: NOT_EXIST_TABLE"); 
            } end

        } end


    } end
}