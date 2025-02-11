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
#include "fmt/core.h"
#include "fmt/color.h"
#include "fmt/format.h"
#include "fmt/os.h"
#include "fmt/ranges.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>


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
             * @brief read text with whitespaces
             * 
             * @param text the whole text we need to parse
             * @param is_first_col_header check if the first column should be used as column or not
             * @param v_hdrs provided headers
             */
            void read_text_whitespace(std::string const& text, bool is_first_col_header = true, std::shorts::V_string v_hdrs = {});

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

            /**
             * @brief add a column to the end of the dataframe based on a provided vector of string
             * 
             * @param v_strs vector of values to add  
             * @param hdr given header name (default new: new_col), if the header is ther it would modified the header name
             */
            void add_col(std::shorts::V_string const& v_values, std::string hdr = "new_col");

           /**
            * @brief dd a column to the end of the dataframe based on a provided value for all rows
            * 
            * @param value value to be add to all rows 
            * @param hdr given header name (default new: new_col), if the header is ther it would modified the header name
            */
            void add_col_of(std::string const& value, std::string hdr = "new_col");
            
            /**
             * @brief check if headers are the same then append the values
             * 
             * @param v_dfs 
             */
            void append(std::vector<DataFrame>&& v_dfs);

            /**
             * @brief to clear headers and data
             * 
             */
            void clear();

            /**
             * @brief subscript operator
             * 
             * @param hdr 
             * @return std::shorts::V_string 
             */
            std::shorts::V_string& operator[](std::string hdr);

            /**
             * @brief write the dataframe in a file with given path and delimiter  
             * 
             * @param path 
             * @param delimiter 
             */
            void write(std::string_view path, char delimiter = ',');

            /**
             * @brief saving dataframe in comma separted format file
             * 
             * @param path 
             */
            void save_as_csv(std::string_view path);
        
        private:
            std::shorts::Data data;
            std::shorts::V_string headers;

            std::shorts::V_string read_lines(std::string_view path);
            std::shorts::V_string read_lines(std::string const& text);
            std::shorts::V_string parse_line_whitespace(std::string const& line);
            std::shorts::V_string parse_line(std::string const& line, char delim);
            std::shorts::V_string parse_line(std::string const& line, std::shorts::V_pair_ints const& v_cols_start_ends);
            void fill_data_whitespace(std::shorts::V_string const& v_lines, bool is_first_col_header = true, std::shorts::V_string v_hdrs = {});
            void fill_data(std::shorts::V_string const& v_lines, char delim = ',', bool is_first_col_header = true, std::shorts::V_string v_hdrs = {});
            void fill_data(std::shorts::V_string const& v_lines, std::shorts::V_pair_ints const& v_cols_start_length, bool is_first_col_header = true, std::shorts::V_string v_hdrs = {});
            void insert_col(std::shorts::V_string const& values, std::string hdr);
    };
    
}