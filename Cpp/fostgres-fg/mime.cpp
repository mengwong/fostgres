/*
    Copyright 2016-2017 Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fostgres/fg/contains.hpp>
#include <fostgres/fg/mime.hpp>
#include <fost/csj.parser.hpp>
#include <fost/insert>
#include <fost/test>


namespace {
    std::vector<unsigned char> body_data(const fostlib::mime &body) {
        std::vector<unsigned char> data;
        for ( const auto &part : body ) {
            data.insert(data.end(),
                reinterpret_cast<const char *>(part.first),
                reinterpret_cast<const char *>(part.second));
        }
        return data;
    }
    fg::json mime_to_json(const fostlib::mime &body) {
        auto data = body_data(body);
        return fostlib::json::parse(
            fostlib::coerce<fostlib::string>(
                fostlib::coerce<fostlib::utf8_string>(data)));
    }
}


std::unique_ptr<fostlib::binary_body> fg::mime_from_argument(
    frame &stack, const json &data
) {
    std::unique_ptr<fostlib::binary_body> body;
    if ( data.isatom() || data.isarray() ) {
        auto filename = fostlib::coerce<boost::filesystem::path>(stack.resolve_string(data));
        auto filedata = fostlib::utf::load_file(filename);
        body.reset(new fostlib::binary_body(
            filedata.std_str().c_str(), filedata.std_str().c_str() + filedata.std_str().length()));
        body->headers().set("Content-Type", fostlib::urlhandler::mime_type(filename));
    } else {
        auto bodydata = fostlib::json::unparse(data, false);
        body.reset(new fostlib::binary_body(
            bodydata.std_str().c_str(), bodydata.std_str().c_str() + bodydata.std_str().length()));
        body->headers().set("Content-Type", "application/json");
    }
    return body;
}


void fg::assert_comparable(const fostlib::mime &actual, const fostlib::mime &expected) {
//     if ( actual.headers()["Content-Type"].value() != expected.headers()["Content-Type"].value() ) {
//         fostlib::exceptions::not_implemented error(__func__, "Content-Type mismatch");
//         fostlib::insert(error.data(), "actual", actual.headers()["Content-Type"].value());
//         fostlib::insert(error.data(), "expected", expected.headers()["Content-Type"].value());
//         throw error;
//     }
    if ( actual.headers()["Content-Type"].value() == "application/json" ) {
        auto actual_body = mime_to_json(actual);
        auto expected_body = mime_to_json(expected);
        auto contains = fg::contains(actual_body, expected_body);
        if ( contains ) {
            fostlib::exceptions::test_failure error("Mismatched response body", __FILE__, __LINE__);
            fostlib::insert(error.data(), "expected", expected_body);
            fostlib::insert(error.data(), "actual", actual_body);
            fostlib::insert(error.data(), "mismatch", "path", contains.value());
            fostlib::insert(error.data(), "mismatch", "expected", expected_body[contains.value()]);
            fostlib::insert(error.data(), "mismatch", "actual", actual_body[contains.value()]);
            throw error;
        }
    } else if ( actual.headers()["Content-Type"].value() == "text/plain" ) {
        /// This should be CSJ
        auto actual_data = body_data(actual);
        fostlib::csj::parser actual_body{fostlib::utf::u8_view(actual_data)};
        auto actual_iter = actual_body.begin(), actual_end = actual_body.end();
        auto expected_body = mime_to_json(expected);
        for ( auto row : expected_body["rows"] ) {
            if ( row.size() != expected_body["columns"].size() ) {
                throw fostlib::exceptions::not_implemented(__func__,
                    "Test data row has the wrong number of columns", row);
            }
            fostlib::json expected_row;
            for ( std::size_t index{}; index < expected_body["columns"].size(); ++index ) {
                fostlib::insert(expected_row, expected_body["columns"][index], row[index]);
            }
            if ( actual_iter == actual_end ) {
                throw fostlib::exceptions::not_implemented(__func__,
                    "Found extra row in the expected data", expected_row);
            }
            auto actual_json = actual_iter.as_json();
            auto contains = fg::contains(actual_json, expected_row);
            if ( contains ) {
                fostlib::exceptions::test_failure error("Mismatched response body", __FILE__, __LINE__);
                fostlib::insert(error.data(), "expected", expected_row);
                fostlib::insert(error.data(), "actual", actual_json);
                fostlib::insert(error.data(), "mismatch", "path", contains.value());
                fostlib::insert(error.data(), "mismatch", "expected", expected_row[contains.value()]);
                fostlib::insert(error.data(), "mismatch", "actual", actual_json[contains.value()]);
                throw error;
            }
            ++actual_iter;
        }
        if ( actual_iter != actual_end ) {
            throw fostlib::exceptions::not_implemented(__func__,
                "Found extra row in the actual data", actual_iter.as_json());
        }
    } else {
        throw fostlib::exceptions::not_implemented(__func__,
            "MIME type", actual.headers()["Content-Type"].value());
    }
}

