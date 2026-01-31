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

#include "appError.hpp"

namespace xk
{
    appError::appError(std::string err_str)
        : err_str_(err_str)
    {

    }

    appError::appError(std::string err_location, std::string err_str)
    {
        std::stringstream strstrm;
        strstrm << "[" << err_location << "] " << err_str;
        err_str_ = strstrm.str();
    }

    appError::appError(std::string err_str, int error_number)
    {
        std::stringstream strstrm;
        strstrm << err_str << " (ERROR: " << error_number << " - " << std::strerror(error_number) << ")";
        err_str_ = strstrm.str();
    }

    appError::appError(std::string err_location, std::string err_str, int error_number)
    {
        std::stringstream strstrm;
        strstrm << "[" << err_location << "] " << err_str << " (ERROR: " << error_number << " - " << std::strerror(error_number) << ")";
        err_str_ = strstrm.str();
    }

    const char* appError::what() const throw()
    {
        return err_str_.c_str();
    }
    
    void appError::redirectStdErrToFile(std::string err_outfile)
    {
        freopen (err_outfile.c_str(), "w", stderr);
    }
}