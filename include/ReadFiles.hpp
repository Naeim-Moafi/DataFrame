/**
 * @file ReadFiles.hpp
 * @author Naeim Moafinejad (snmoafinejad@iimcb.gov.pl, s.naeim.moafi.n@gmail.com)
 * @brief A class for reading files with different delimiters
 * @version 0.1
 * @date 2024-10-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <any>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "fmt/core.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "fmt/color.h"


namespace std
{
    /**
     * @brief namespace for introducting shortnames
     * 
     */
    namespace shorts
    {
        using V_string = vector<string>;
        using VV_string = vector<V_string>;
        using V_any = vector<any>;
        using VV_any =vector<vector<any>>;
        using Data = unordered_map<string, V_string>;
        using V_double = vector<double>;
        using V_int = vector<int>;
        using V_pair_ints = std::vector<std::pair<int, int>>;
    }   
}

namespace DF
{
    /**
     * @brief DataFrame is class for parsing data in a given file
     * with a give delimeter (default is comma ',').
     * all data will be saved as string wich user can later covert to desired type
     * 
     */
    class DataFrame
    {
        public:
            
            /**
             * @brief number of rows
             * 
             */
            unsigned long long n_rows;

            /**
             * @brief number of columns
             * 
             */
            unsigned long long n_cols;            

            /**
             * @brief an unorderd maps for missing values
             * 
             */
            std::unordered_map<int, int> mising_values;

            /**
             * @brief Get the number of rows of a given data
             * 
             * @return int number of rows
             */
            
            int get_n_rows() const;
            /**
             * @brief Get the number cols of a given data
             * 
             * @return int number of columns
             */
            int get_n_cols() const;

            /**
             * @brief Set the headers with user provided vector of strings
             * 
             * @param v_hdrs provided headers from users
             */
            void set_headers(std::shorts::V_string const& v_hdrs);

            /**
             * @brief Get the headers
             * 
             * @return std::shorts::V_string headers
             */
            std::shorts::V_string get_headers() const;


            std::shorts::V_string get_by_header(std::string const& hdr);

            /**
             * @brief to copy current data into new data frame
             * 
             * @return DataFrame new data frame
             */
            DataFrame copy() const;

            /**
             * @brief copy the requested data by provided headers name as vector of strings into a new data frame
             * 
             * @param v_hdrs 
             * @return DataFrame new data frame
             */
            DataFrame copy_by_headers(std::shorts::V_string const& v_hdrs);

            /**
             * @brief get a given header and only keep the first occurance and remove the remaning rows
             * 
             * @param hdr header to check the duplication 
             */
            void remove_duplications(std::string const& hdr);

            /**
             * @brief read files 
             * 
             * @param path path to input file
             * @param delim delimiter for parsing the input file
             * @param is_first_col_header boolean
             */
            void read_files(std::string_view path, char delim = ',', bool is_first_col_header = true, std::shorts::V_string v_hdrs = {});

            /**
             * @brief 
             * 
             * @param path std::string_view
             * @param v_cols_length 
             * @param is_first_col_header 
             */
            void read_text(std::string const& text,std::shorts::V_pair_ints const& v_cols_start_length, bool is_first_col_header = true, std::shorts::V_string v_hdrs = {});

            /**
             * @brief print n first rows off all columns
             * 
             * @param n 
             */
            void head(unsigned long long n = 5);

            /**
             * @brief swap two columns with their header with each others
             * 
             * @param first_hdr std::string
             * @param second_hdr std::string
             */
            void swap_cols_pos(std::string first_hdr, std::string second_hdr);
        
        private:
            std::shorts::Data data;
            std::shorts::V_string headers;

            std::shorts::V_string read_lines(std::string_view path);
            std::shorts::V_string read_lines(std::string const& text);
            std::shorts::V_string parse_line(std::string const& line, char delim);
            std::shorts::V_string parse_line(std::string const& line, std::shorts::V_pair_ints const& v_cols_start_ends);
            void fill_data(std::shorts::V_string const& v_lines, char delim = ',', bool is_first_col_header = true, std::shorts::V_string v_hdrs = {});
            void fill_data(std::shorts::V_string const& v_lines, std::shorts::V_pair_ints const& v_cols_start_length, bool is_first_col_header = true, std::shorts::V_string v_hdrs = {});
    };
    
}