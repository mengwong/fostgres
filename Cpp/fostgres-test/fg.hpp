/*
    Copyright 2016 Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/file>


namespace fg {


    class program {
    public:
        /// Construct an empty program that errors when run
        program();
        /// Parse the requested program
        explicit program(const boost::filesystem::path &);

        /// Execute this program
        void operator () () const;
    };


}

