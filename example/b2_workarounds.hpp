// Copyright 2014 Renato Tegon Forti, Antony Polukhin.
// Copyright 2015 Antony Polukhin.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DLL_EXAMPLE_COMMON_B2_WORKAROUNDS_HPP
#define BOOST_DLL_EXAMPLE_COMMON_B2_WORKAROUNDS_HPP

#include <boost/filesystem.hpp>
#include <cctype>

namespace b2_workarounds {


inline boost::filesystem::path drop_version(const boost::filesystem::path& lhs) {
    boost::filesystem::path ext = lhs.filename().extension();
    if (ext.native().size() > 1 && std::isdigit(ext.string()[1])) {
        ext = lhs;
        ext.replace_extension().replace_extension().replace_extension();
        return ext;
    }

    return lhs;
}


// This ugly struct is required to drop library version from shared library generated by b2.
struct argv_to_path_guard {
    const boost::filesystem::path original_;
    const boost::filesystem::path version_dropped_;
    const std::string just_path_;
    const bool same_;


    static inline boost::filesystem::path first_from_argv(int argc, char* argv[]) {
        BOOST_ASSERT(argc > 1);
        (void)argc;

        return argv[1];
    }

    static inline boost::filesystem::path drop_b2_deco(const boost::filesystem::path& in) {
        std::size_t pos = in.filename().string().find("-");
        boost::filesystem::path res = in.parent_path() / in.filename().string().substr(0, in.filename().string().find("-"));
        if (pos != std::string::npos) {
            res += in.extension();
        }
        return res;
    }

    inline explicit argv_to_path_guard(int argc, char* argv[])
        : original_(first_from_argv(argc, argv))
        , version_dropped_( drop_b2_deco(drop_version(original_)) )
        , just_path_( version_dropped_.parent_path().string() )
        , same_(version_dropped_ == original_)
    {
        if (!same_) {
            boost::system::error_code ignore;
            boost::filesystem::remove(version_dropped_, ignore);
            boost::filesystem::copy(original_, version_dropped_, ignore);
        }

        argv[1] = const_cast<char*>(just_path_.c_str());
    }

    inline ~argv_to_path_guard() {
        if (!same_) {
            boost::system::error_code ignore;
            boost::filesystem::remove(version_dropped_, ignore);
        }
    }
};


inline bool is_shared_library(const boost::filesystem::path& p) {
    const std::string s = p.string();
    return (s.find(".dll") != std::string::npos || s.find(".so") != std::string::npos || s.find(".dylib") != std::string::npos)
            && s.find(".lib") == std::string::npos
            && s.find(".exp") == std::string::npos
            && s.find(".pdb") == std::string::npos
            && s.find(".manifest") == std::string::npos
            && s.find(".rsp") == std::string::npos
            && s.find(".obj") == std::string::npos
            && s.find(".a") == std::string::npos;
}

} // namespace b2_workarounds

#endif // BOOST_DLL_EXAMPLE_COMMON_B2_WORKAROUNDS_HPP

