#include <algorithm>
#include <exception>
#include <fmt/os.h>
#include <fstream>
#include <iostream>
#include "ReadFiles.hpp"
#include <set>
#include <sstream>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

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

std::shorts::V_string DF::DataFrame::read_lines(std::string const& text)
{
    std::istringstream iss(text);

    std::string line;
    std::shorts::V_string v_strs;


    while(std::getline(iss, line))
    {
        if(line.size() == 0) continue;
        line.erase(std::remove(line.begin(), line.end(), '\r'),line.end());
        v_strs.emplace_back(line);
    }

    return v_strs;
}

std::shorts::V_string DF::DataFrame::parse_line_whitespace(std::string const& line)
{
    std::shorts::V_string v_strs;
    std::istringstream iss(line);
    std::string cell;

    while(iss >> cell)
    {
        cell.erase(std::remove(cell.begin(), cell.end(), '\"'), cell.end());
        v_strs.emplace_back(cell);
    }

    return v_strs;
}



std::shorts::V_string DF::DataFrame::parse_line(std::string const& line,char delim)
{
    std::shorts::V_string v_strs;

    std::istringstream iss(line);
    std::string cell;

    while(std::getline(iss, cell, delim))
    {
        cell.erase(std::remove(cell.begin(), cell.end(), '\"'), cell.end());
        cell.erase(std::remove(cell.begin(), cell.end(), ' '), cell.end());
        v_strs.emplace_back(cell);
    }

    return v_strs;
}

std::shorts::V_string DF::DataFrame::parse_line(std::string const& line,std::shorts::V_pair_ints const& v_cols_start_ends)
{
    std::shorts::V_string v_strs;

    for(auto const& [start, end] : v_cols_start_ends)
    {
        std::string cell;
        try
        {
            cell = line.substr(start, end);
        }
        catch(...)
        {
            cell = "NA";
        }
        
        // auto cell = line.substr(start, end);
        cell.erase(std::remove(cell.begin(), cell.end(), '\"'), cell.end());
        cell.erase(std::remove(cell.begin(), cell.end(), ' '), cell.end());
        v_strs.push_back(cell);
    }

    return v_strs;
}

void DF::DataFrame::fill_data_whitespace(std::shorts::V_string const& v_lines, bool is_first_col_header, std::shorts::V_string v_hdrs)
{
    std::shorts::VV_string vv_strs;

    for(auto const& line : v_lines)
    {
        std::shorts::V_string v_str_tmp = parse_line_whitespace(line);
        vv_strs.emplace_back(v_str_tmp);
    }

    //initialize n_rows and n_cols
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
            }

            values.emplace_back(vv_strs[i_row][i_col]);

            // check for the missig values
            // missing values are empty string, NA, and NAN
            if(vv_strs[i_row][i_col] == ""   || 
               vv_strs[i_row][i_col] == "NA" || 
               vv_strs[i_row][i_col] == "NAN")
            {
                mising_values.insert({i_row, i_col});           
            }
        }

        data[headers[i_col]] = values;
    }
}

void DF::DataFrame::fill_data(std::shorts::V_string const& lines, char delim, bool is_first_col_header, std::shorts::V_string v_hdrs)
{
    std::shorts::VV_string vv_strs;
    
    for(auto const& line : lines)
    {
        std::shorts::V_string v_str_tmp = parse_line(line, delim);
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
            }

            values.emplace_back(vv_strs[i_row][i_col]);

            // check for the missig values
            // missing values are empty string, NA, and NAN
            if(vv_strs[i_row][i_col].empty() || 
               vv_strs[i_row][i_col] == "NA" || 
               vv_strs[i_row][i_col] == "NAN")
            {
                mising_values.insert({i_row, i_col});           
            }
        }

        data[headers[i_col]] = values;
    }
}

void DF::DataFrame::fill_data(std::shorts::V_string const& lines, std::shorts::V_pair_ints const& v_cols_start_length, bool is_first_col_header, std::shorts::V_string v_hdrs)
{
    std::shorts::VV_string vv_strs;
    for(auto const& line : lines)
    {
        auto v_str_tmp = parse_line(line, v_cols_start_length);
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
            }

            values.emplace_back(vv_strs[i_row][i_col]);

            // check for the missig values
            // missing values are empty string, NA, and NAN
            if(vv_strs[i_row][i_col].empty() || 
               vv_strs[i_row][i_col] == "NA" || 
               vv_strs[i_row][i_col] == "NAN")
            {
                mising_values.insert({i_row, i_col});           
            }
        }

        data[headers[i_col]] = values;
    }
}

void DF::DataFrame::read_files(std::string_view path, char delim, bool is_first_col_header, std::shorts::V_string v_hdrs)
{
    auto lines = read_lines(path);
    fill_data(lines, delim, is_first_col_header, v_hdrs);
}

void DF::DataFrame::read_text(std::string const& text,std::shorts::V_pair_ints const& v_cols_start_length, bool is_first_col_header, std::shorts::V_string v_hdrs)
{
    auto lines = read_lines(text);
    fill_data(lines, v_cols_start_length, is_first_col_header, v_hdrs);
}

void DF::DataFrame::read_text_whitespace(std::string const& text, bool is_first_col_header, std::shorts::V_string v_hdrs)
{
    auto lines = read_lines(text);
    fill_data_whitespace(lines, is_first_col_header, v_hdrs);
}

void DF::DataFrame::head(unsigned long long n)
{
    if(n > n_rows)
    {
        fmt::print(fg(fmt::color::yellow),"Warning: number of row requested ({}) to be printed is bigger than number of available rows in data ({})\n",
        n, data[headers[0]].size());
        if(n_rows > 5) n = 5;
        else n = n_rows;
    }

    struct winsize w;
    
    // Get the terminal size
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        std::cerr << "Error getting terminal size\n";
        exit(EXIT_FAILURE);
    }

    int col_width = (w.ws_col - 30)/(headers.size() + 1);
    if(col_width < 5)
    {
        fmt::print(fg(fmt::color::yellow), "to many data to show. the printed data may be not be informative.\n");
    }
    

    fmt::print("{:{}}|", " ", col_width);
    for(auto const& curr_hdr : headers)
    {
        fmt::print(fmt::emphasis::bold | fg(fmt::color::green), "{:^{}}|", curr_hdr.substr(0,col_width), col_width); 
    }

    std::string sub_separator(col_width, '-');
    sub_separator += '+';
    
    std::string separator;
    separator.reserve(n_cols * sub_separator.size()+1);
    
    for (size_t i = 0; i < n_cols+1; ++i) 
    {
        separator += sub_separator; 
    }
    
    fmt::println("\n{}",std::string(separator));

    // std::cout << '\n';

    if(n <= 10)
    {
        for(unsigned long long i{0}; i < n; ++i)
        {
            fmt::print(fg(fmt::color::green),"|{:^{}}|", std::to_string(i+1).substr(0,col_width), col_width - 1);
            for(auto const& curr_hdr : headers)
            {
                const auto& value = data[curr_hdr][i];
                if (value.empty() || value == "NA" || value == "NAN")
                {
                    fmt::print(bg(fmt::color::red),"{:^{}}", data[curr_hdr][i].substr(0,col_width), col_width);
                    std::cout << "|";
                }
                else
                {
                    fmt::print("{:^{}}|", data[curr_hdr][i].substr(0,col_width), col_width);
                }
                
            }

            std::cout << "\n";
        }
    }
    else
    {
        for(unsigned long long i{0}; i < 5; ++i)
        {
            fmt::print(fg(fmt::color::green),"|{:^{}}|", std::to_string(i+1).substr(0,col_width), col_width - 1);
            for(auto const& curr_hdr : headers)
            {
                const auto& value = data[curr_hdr][i];
                if (value.empty() || value == "NA" || value == "NAN")
                {
                    fmt::print(bg(fmt::color::red),"{:^{}}", data[curr_hdr][i].substr(0,col_width), col_width);
                    std::cout << "|";
                }
                else
                {
                    fmt::print("{:^10}|", data[curr_hdr][i].substr(0,col_width), col_width );
                }
                
            }

            std::cout << "\n";
        }

        std::cout << "\n\t.\n\t.\n\t.\n\n";

        for(unsigned long long i{n-5}; i < n; ++i)
        {
            fmt::print(fg(fmt::color::green),"|{:^{}}|", std::to_string(i+1).substr(0,col_width), col_width - 1);
            for(auto const& curr_hdr : headers)
            {
                const auto& value = data[curr_hdr][i];
                if (value.empty() || value == "NA" || value == "NAN")
                {
                    fmt::print(bg(fmt::color::red),"{:^{}}", data[curr_hdr][i].substr(0,col_width), col_width);
                    std::cout << "|";
                }
                else
                {
                    fmt::print("{:^{}}|", data[curr_hdr][i].substr(0,col_width), col_width);
                }
                
            }

            std::cout << "\n";
        }
    }
    fmt::println("{}",std::string(separator));
}

std::shorts::V_string DF::DataFrame::get_headers() const
{
    return headers;
}

void DF::DataFrame::set_headers(std::shorts::V_string const& v_hdrs)
{
    headers = v_hdrs;
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

void DF::DataFrame::insert_col(std::shorts::V_string const& values, std::string hdr)
{
    auto [_it, inserted] = data.insert({hdr, values});
    
    int n = 1;
    std::string original_hdr = hdr;

    while (!inserted)
    {
        hdr = fmt::format("{}_{}", original_hdr, n++);
        std::tie(_it, inserted) = data.insert({hdr, values});
    }
    headers.push_back(hdr);
    n_cols++;
}

void DF::DataFrame::add_col(std::shorts::V_string const& values, std::string hdr)
                           
{
    insert_col(values, hdr);
}

void DF::DataFrame::add_col_of(std::string const& value, std::string hdr)
{
    std::shorts::V_string values(data[headers[0]].size(), value);
    insert_col(values, hdr);
}

void DF::DataFrame::clear()
{
    headers.clear();
    data.clear();
}

void DF::DataFrame::append(std::vector<DF::DataFrame>&& v_dfs)
{
    if(v_dfs.empty()) return;

    for(auto& curr_df : v_dfs)
    {
        if(headers == curr_df.get_headers())
        {
            for(auto curr_hdr : headers)
            {
                data[curr_hdr].insert(data[curr_hdr].end(), curr_df.data.at(curr_hdr).begin(), curr_df.data.at(curr_hdr).end());
            }
        }
        curr_df.clear();
    }
    n_rows = data[headers[0]].size();
    n_cols = headers.size();
}

std::shorts::V_string&  DF::DataFrame::operator[](std::string hdr)
{
    return data[hdr];
}

void DF::DataFrame::write(std::string_view path, char delimiter)
{
    fmt::ostream out = fmt::output_file(path.data());

    for(auto const& curr_hdr : headers)
    {
        out.print("{}{}", curr_hdr.substr(0,10), delimiter); 
    }
    out.print("\n");

    for(unsigned long long i{0}; i < n_rows; ++i)
    {
        for(auto const& curr_hdr : headers)
        {
            const auto& value = data[curr_hdr][i];
            out.print("{}{}", value, delimiter);
            
        }

        out.print("\n");
    }
}

void DF::DataFrame::save_as_csv(std::string_view path)
{
    write(path /*,delimiter=','*/);
}


int main()
{
    std::string software_name = R"( ____        _        _____                         
|  _ \  __ _| |_ __ _|  ___| __ __ _ _ __ ___   ___ 
| | | |/ _` | __/ _` | |_ | '__/ _` | '_ ` _ \ / _ \
| |_| | (_| | || (_| |  _|| | | (_| | | | | | |  __/
|____/ \__,_|\__\__,_|_|  |_|  \__,_|_| |_| |_|\___|)";

        std::cout << software_name << std::endl << std::endl;
    // DF::DataFrame df;


    // df.read_files("test.txt", '\t');
    // df.head();
    // std::cout << '\n';

    // auto df2 = df.copy_by_headers({"id", "age", "disease"});
    // df2.head();
    // std::cout << '\n';

    // df2.swap_cols_pos("age", "disease");
    // df2.head();
    // std::cout << '\n';

    // df.read_files("test.txt", '\t', false);
    // df.head();
    // std::cout << '\n';

    // df.read_files("test.txt", '\t', false, {"1.0000", "2.0000", "3.0000", "4.0000", "5.0000"});
    // df.head();
    // std::cout << '\n';

    // df.head(600);
    // std::cout << '\n';

    DF::DataFrame df3;
    std::vector<std::string> v_hdrs {"group_PDB", "id", "label_atom_id", "alt_id", "label_comp_id", "label_asym_id", 
                                     "label_seq_id", "Cartn_x", "Cartn_y", "Cartn_z", "occupancy", 
                                     "B_iso_or_equiv", "type_symbol", "charge"};

    std::shorts::V_pair_ints fields_intervals = {{0,6}, {6,5}, {12,4}, {16,1}, {17,3}, {20,2}, {22,4}, {30,8}, {38,8}, {46,8}, {54,6}, {60,6}, {76,2}, {78,2}};

    // std::string text{"HETATM    1  PG  GTP A   1      24.181  32.064  27.670  0.10 24.73           P  \nHETATM    2  O1G GTP A   1      24.342  33.433  27.064  0.10 37.56           O  \nHETATM    3  O2G GTP A   1      24.519  32.013  29.136  0.10 32.46           O  \n"};
    std::string text = R"(HETATM    1  PG  GTP A   1      24.181  32.064  27.670  0.10 24.73           P  
HETATM    2  O1G GTP A   1      24.342  33.433  27.064  0.10 37.56           O  
HETATM    3  O2G GTP A   1      24.519  32.013  29.136  0.10 32.46           O  
HETATM    4  O3G GTP A   1      25.048  31.062  26.907  1.00 47.91           O  
HETATM    5  O3B GTP A   1      22.644  31.665  27.526  1.00 33.11           O )";
    
    df3.read_text(text, fields_intervals, false, v_hdrs);
    df3.head();

    std::string text2 = R"(HETATM 1   P  PG    . GTP A 1 1  ? 24.181  32.064 27.670 0.10 24.73 ? 1   GTP A PG    1 
HETATM 2   O  O1G   . GTP A 1 1  ? 24.342  33.433 27.064 0.10 37.56 ? 1   GTP A O1G   1 
HETATM 3   O  O2G   . GTP A 1 1  ? 24.519  32.013 29.136 0.10 32.46 ? 1   GTP A O2G   1 
HETATM 4   O  O3G   . GTP A 1 1  ? 25.048  31.062 26.907 1.00 47.91 ? 1   GTP A O3G   1 
HETATM 5   O  O3B   . GTP A 1 1  ? 22.644  31.665 27.526 1.00 33.11 ? 1   GTP A O3B   1 
HETATM 6   P  PB    . GTP A 1 1  ? 21.944  30.914 26.290 1.00 30.44 ? 1   GTP A PB    1 
HETATM 7   O  O1B   . GTP A 1 1  ? 20.481  30.888 26.504 1.00 30.75 ? 1   GTP A O1B   1 
HETATM 8   O  O2B   . GTP A 1 1  ? 22.544  29.597 26.037 1.00 31.25 ? 1   GTP A O2B   1 
HETATM 9   O  O3A   . GTP A 1 1  ? 22.230  31.881 25.061 1.00 27.41 ? 1   GTP A O3A   1 
HETATM 10  P  PA    . GTP A 1 1  ? 21.357  32.886 24.136 1.00 28.14 ? 1   GTP A PA    1 
HETATM 11  O  O1A   . GTP A 1 1  ? 22.257  33.379 23.051 1.00 29.88 ? 1   GTP A O1A   1 
HETATM 12  O  O2A   . GTP A 1 1  ? 20.694  33.906 24.985 1.00 28.72 ? 1   GTP A O2A   1 
HETATM 13  O  "O5'" A GTP A 1 1  ? 20.284  31.879 23.543 0.50 23.11 ? 1   GTP A "O5'" 1 
HETATM 14  O  "O5'" B GTP A 1 1  ? 20.244  31.934 23.525 0.50 23.80 ? 1   GTP A "O5'" 1 
HETATM 15  C  "C5'" A GTP A 1 1  ? 19.131  32.316 22.825 0.50 21.95 ? 1   GTP A "C5'" 1 
HETATM 16  C  "C5'" B GTP A 1 1  ? 19.049  32.533 22.993 0.50 20.46 ? 1   GTP A "C5'" 1 
HETATM 17  C  "C4'" A GTP A 1 1  ? 18.305  31.114 22.431 0.50 18.93 ? 1   GTP A "C4'" 1 
HETATM 18  C  "C4'" B GTP A 1 1  ? 18.162  31.423 22.482 0.50 19.32 ? 1   GTP A "C4'" 1 
HETATM 19  O  "O4'" A GTP A 1 1  ? 19.032  30.287 21.495 0.50 16.95 ? 1   GTP A "O4'" 1 
HETATM 20  O  "O4'" B GTP A 1 1  ? 18.804  30.735 21.386 0.50 17.14 ? 1   GTP A "O4'" 1 
HETATM 21  C  "C3'" A GTP A 1 1  ? 17.949  30.186 23.587 0.50 16.54 ? 1   GTP A "C3'" 1 
HETATM 22  C  "C3'" B GTP A 1 1  ? 17.857  30.361 23.529 0.50 18.33 ? 1   GTP A "C3'" 1 
HETATM 23  O  "O3'" A GTP A 1 1  ? 16.747  30.614 24.223 0.50 17.14 ? 1   GTP A "O3'" 1 
HETATM 24  O  "O3'" B GTP A 1 1  ? 16.664  30.669 24.225 0.50 18.17 ? 1   GTP A "O3'" 1 
HETATM 25  C  "C2'" A GTP A 1 1  ? 17.816  28.834 22.929 0.50 16.16 ? 1   GTP A "C2'" 1 )";

    DF::DataFrame df4;

    df4.read_text_whitespace(text2, /*is_first_cols_header=*/false /*headers=default*/);
    df4.head();
    
    


    return EXIT_SUCCESS;
}
