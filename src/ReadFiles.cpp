#include "ReadFiles.hpp"


#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>

int DF::DataFrame::get_n_cols() const
{
    return n_cols;
}

int DF::DataFrame::get_n_rows() const
{
    return n_rows;
}

std::shorts::V_string DF::DataFrame::read_lines(std::string_view path)
{
    std::ifstream ifs(path.data());

    if(ifs.fail())
    {
        throw std::runtime_error(fmt::format(fg(fmt::color::red), "Error: unable to read file {}.\nPlease check your input.", path));
        exit(EXIT_FAILURE);
    }


    std::string line;
    std::shorts::V_string v_strs;


    while(std::getline(ifs, line))
    {
        if(line.size() == 0) continue;
        line.erase(std::remove(line.begin(), line.end(), '\r'),line.end());
        v_strs.emplace_back(line);
    }

    return v_strs;
}


void DF::DataFrame::fill_data(std::shorts::V_string const& lines, char delim, bool is_first_col_header, std::shorts::V_string v_hdrs)
{
    std::shorts::VV_string vv_strs;
    
    for(auto const& line : lines)
    {
        std::istringstream iss(line);
        std::string cell;
        std::shorts::V_string v_str_tmp;

        while(std::getline(iss, cell, delim))
        {
            cell.erase(std::remove(cell.begin(), cell.end(), '\"'), cell.end());
            v_str_tmp.emplace_back(cell);
        }
        vv_strs.emplace_back(v_str_tmp);
    }

    // init n_rows and n_cols
    n_rows = vv_strs.size();
    n_cols = vv_strs[0].size();

    headers.resize(n_cols);

    // initializing headers
    for(unsigned long long i_col{0}; i_col < n_cols; ++i_col)
    {
        if(is_first_col_header)
        {
            headers[i_col] = vv_strs[0][i_col];
        }
        else if(v_hdrs.size() > 0)
        {
            if(v_hdrs.size() == n_cols)
            {
                headers[i_col] = v_hdrs[i_col];
            }
            else
            {
                throw std::runtime_error(fmt::format(fg(fmt::color::red), "Error: number of provided headers does not match with the number of columns in the data"));
            }
        }
        else
        {
            headers[i_col] = std::to_string(i_col + 1);
        }
    }

    for(unsigned long long i_col{0}; i_col < n_cols; ++i_col)
    {
        std::shorts::V_string values;
        for(unsigned long long i_row{is_first_col_header}; i_row < n_rows; ++i_row)
        {
            if(vv_strs[i_row].size() != n_cols)
            {
                throw std::runtime_error(fmt::format(fg(fmt::color::red), "Error: inconsistent number of columns, check row {}", i_row + 1));
                exit(EXIT_FAILURE);
            }

            values.emplace_back(vv_strs[i_row][i_col]);

            // check for the missig values
            // missing values are empty string, NA, and NAN
            if(vv_strs[i_row][i_col] == "" || vv_strs[i_row][i_col] == "NA" || vv_strs[i_row][i_col] == "NAN")
            {
                mising_values.insert({i_row, i_col});           
            }
        }

        data[headers[i_col]] = values;
    }
}

void DF::DataFrame::read_files(std::string_view path, char delim, bool is_first_col_header, std::shorts::V_string v_hdrs)
{
    auto v_strs = read_lines(path);
    fill_data(v_strs, delim, is_first_col_header, v_hdrs);
}

void DF::DataFrame::head(unsigned long long n)
{
    if(n > n_rows)
    {
        fmt::print(fg(fmt::color::yellow),"Warning: number of columns requested ({}) to be printed is bigger than number of available rows in data ({})\n",
        n, data[headers[0]].size());
        if(n_rows > 5) n = 5;
        else n = n_rows;
    }

    for(auto const& curr_hdr : headers)
    {
        fmt::print(fmt::emphasis::bold | fg(fmt::color::green), "{:10} ", curr_hdr);
    }

    std::cout << '\n';

    for(unsigned long long i{0}; i < n; ++i)
    {
        for(auto const& curr_hdr : headers)
        {
            if(data[curr_hdr][i] == "" || data[curr_hdr][i] == "NA" || data[curr_hdr][i] == "NAN")
            {
                fmt::print(bg(fmt::color::red),"{:10} ", data[curr_hdr][i]);
            }
            else
            {
                fmt::print("{:10} ", data[curr_hdr][i]);
            }
            
        }

        std::cout << "\n";
    }
}

std::shorts::V_string DF::DataFrame::get_headers() const
{
    return headers;
}

std::shorts::V_string DF::DataFrame::get_by_header(std::string const& hdr)
{
    return data[hdr];
}

DF::DataFrame DF::DataFrame::copy_by_headers(std::shorts::V_string const& v_hdrs)
{
    DF::DataFrame new_df;
    new_df.n_cols = v_hdrs.size();
    new_df.n_rows = n_rows;
    new_df.headers = v_hdrs;

    for(auto const& hdr : v_hdrs)
    {
        new_df.data[hdr] = data[hdr];
    }  

    return new_df;
}

void DF::DataFrame::swap_cols_pos(std::string first_hdr, std::string second_hdr)
{
    std::string tmp_hdr;
    std::shorts::V_string tmp_vals;
    auto first_it = std::find(headers.begin(), headers.end(), first_hdr);
    auto second_it = std::find(headers.begin(), headers.end(), second_hdr);
    swap(headers[first_it - headers.begin()], headers[second_it - headers.begin()]);

    std::swap(data[first_hdr], data[second_hdr]);
}



int main()
{
    DF::DataFrame df;


    df.read_files("test.txt", '\t');
    df.head();
    std::cout << '\n';

    auto df2 = df.copy_by_headers({"id", "age", "disease"});
    df2.head();
    std::cout << '\n';

    df2.swap_cols_pos("age", "disease");
    df2.head();
    std::cout << '\n';

    df.read_files("test.txt", '\t', false);
    df.head();
    std::cout << '\n';

    df.read_files("test.txt", '\t', false, {"1.0000", "2.0000", "3.0000", "4.0000", "5.0000"});
    df.head();
    std::cout << '\n';

    df.head(600);


    return EXIT_SUCCESS;
}


