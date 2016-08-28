/*
    Copyright 2016 Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fostgres/fg/mime.hpp>
#include <fostgres/fg/fg.hpp>
#include <fostgres/fg/fg.testserver.hpp>


namespace {
    fg::json put(
        fg::frame &stack, fg::json::const_iterator pos, fg::json::const_iterator end
    ) {
        auto viewname = stack.resolve_string(stack.argument("view", pos, end));
        auto path = stack.resolve_string(stack.argument("path", pos, end));
        auto body = stack.argument("body", pos, end);
        auto status = stack.resolve_int(stack.argument("status", pos, end));
        auto response = fg::mime_from_argument(stack, stack.argument("response", pos, end));
        fg::testserver server(stack, viewname);
        auto actual = server.put(stack, path, body);
        if ( actual.second != status ) {
            throw fostlib::exceptions::not_implemented(__func__,
                "Actual resopnse status isn't what was epected", actual.second);
        }
        fg::assert_comparable(*actual.first, *response);
        return fostlib::json();
    }
}


fg::frame::builtin fg::lib::put = ::put;

