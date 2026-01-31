/*
 * Basic Exception Class for C++ Exceptions 
 * Copyright (C) 2024 Koos du Preez
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * CREATED BY: Koos du Preez - kdupreez@hotmail.com
 */
            
#ifndef APP_ERR_HPP
#define APP_ERR_HPP

#include <cstdio>
#include <sstream>
#include <cstring>
#include <exception>

namespace  xk
{

    class appError: public std::exception
    {
        public:
            appError(std::string err_str);
            appError(std::string err_location, std::string err_str);
            
            appError(std::string err_str, int error_number);
            appError(std::string err_location, std::string err_str, int error_number);
            
            virtual const char* what() const throw();
            
            static void redirectStdErrToFile(std::string err_outfile);
            
        private:
            std::string err_str_;
    };
}

#endif /* APP_ERR_HPP */

