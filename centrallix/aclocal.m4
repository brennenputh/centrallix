dnl Test for readline support.  Some versions of readline also require
dnl ncurses, so test for that if necessary.
AC_DEFUN(CENTRALLIX_CHECK_READLINE,
    [
	readline="test"
	AC_CHECK_LIB(readline,
	    readline,
	    [LIBS="$LIBS -lreadline"
	    readline="yes"]
	)
	if test "$readline" = "test"; then
	    unset ac_cv_lib_readline_readline
	    AC_CHECK_LIB(readline,
		readline,
		[LIBS="$LIBS -lreadline -lncurses"
		readline="yes"],,
		-lncurses
	    )
	fi
	if test "$readline" = "test"; then
	    AC_CHECK_FUNCS(readline, readline="yes")
	fi

	if test "$readline" = "yes"; then
	    AC_DEFINE(HAVE_READLINE)
	fi
    ]
)

dnl Test for the Centrallix header and library files.
AC_DEFUN(CENTRALLIX_CHECK_CENTRALLIX,
    [
	AC_ARG_WITH(centrallix-inc,
	    AC_HELP_STRING([--with-centrallix-inc=DIR],
		[Location of centrallix-libs include directory (default is PREFIX/include)]
	    ),
	    centrallix_incdir="$withval",
	    centrallix_incdir="$prefix/include"
	)

 	CFLAGS="$CFLAGS -I$centrallix_incdir"
 	CPPFLAGS="$CPPFLAGS -I$centrallix_incdir"
 	AC_CHECK_HEADER(mtask.h, 
	    [],
 	    AC_MSG_ERROR([Please ensure that Centrallix-libs is installed and use --with-centrallix-inc=DIR to specify the path to the header files])
 	)

	AC_ARG_WITH(centrallix-lib,
	    AC_HELP_STRING([--with-centrallix-lib=DIR],
		[Location of centrallix-libs lib directory (default is PREFIX/lib)]
	    ),
	    centrallix_libdir="$withval",
	    centrallix_libdir="$prefix/lib"
	)

	temp=$LIBS
 	LIBS="$LIBS -L$centrallix_libdir"
 	AC_CHECK_LIB(Centrallix, 
	    mtInitialize,
	    [],
 	    AC_MSG_ERROR([Please ensure that Centrallix-libs is installed and use --with-centrallix-lib=DIR to specify the path to the library])
 	)
	AC_SUBST(CXLIBDIR, $centrallix_libdir)
    ]
)

dnl Define the location of the Centrallix config file
AC_DEFUN(CENTRALLIX_CONF_FILE,
    [
	centrallix_config="$sysconfdir/centrallix.conf"
	if test "$prefix" = "NONE"; then
	    centrallix_config="`echo $centrallix_config | sed -e "s|\\\${prefix}|$ac_default_prefix|"`"
	else
	    centrallix_config="`echo $centrallix_config | sed -e "s|\\\${prefix}|$prefix|"`"
	fi

	AC_DEFINE_UNQUOTED(CENTRALLIX_CONFIG, 
	    "$centrallix_config", 
	    [Location of the centrallix config file]
	)
    ]
)

dnl Define the location of the Centrallix OS tree
AC_DEFUN(CENTRALLIX_OS_DIR,
    [
	AC_ARG_WITH(centrallix-os,
	    AC_HELP_STRING([--with-centrallix-os=FILE],
		[Full path of centrallix OS directory (default is /var/centrallix/os)]
	    ),
	    centrallix_os="$withval",
	    centrallix_os="/var/centrallix/os"
	)

	AC_SUBST(CXOSDIR, $centrallix_os)
    ]
)


dnl Test for the Sybase libraries.
AC_DEFUN(CENTRALLIX_CHECK_SYBASE,
    [
	AC_MSG_CHECKING(if Sybase support is desired)

	AC_ARG_ENABLE(sybase,
	    AC_HELP_STRING([--disable-sybase],
		[disable Sybase support]
	    ),
	    WITH_SYBASE="$enableval", 
	    WITH_SYBASE="yes"
	)
 
	AC_ARG_WITH(sybase-inc,
	    AC_HELP_STRING([--with-sybase-inc=PATH],
		[include path for Sybase headers (default is /opt/sybase/include)]
	    ),
	    sybase_incdir="$withval",
	    sybase_incdir="/opt/sybase/include"
	)
 
	AC_ARG_WITH(sybase-lib,
	    AC_HELP_STRING([--with-sybase-lib=PATH],
		[library path for Sybase libraries (default is /opt/sybase/lib)]
	    ),
	    sybase_libdir="$withval",
	    sybase_libdir="/opt/sybase/lib"
	)
 
	if test "$WITH_SYBASE" = "no"; then
	    AC_MSG_RESULT(no)
	else
	    AC_MSG_RESULT(yes)
 
	    temp=$CPPFLAGS
	    CPPFLAGS="$CPPFLAGS -I$sybase_incdir"
	    AC_CHECK_HEADER(ctpublic.h, 
		WITH_SYBASE="yes",
		WITH_SYBASE="no"
	    )
	    CPPFLAGS="$temp"

	    if test "$WITH_SYBASE" = "yes"; then
		SYBASE_CFLAGS="-I$sybase_incdir"
		temp=$LIBS
		LIBS="$LIBS -L$sybase_libdir"
		AC_CHECK_LIB(ct, ct_connect, WITH_SYBASE_CT="yes", WITH_SYBASE_CT="no", -lcomn -lsybtcl -linsck -lintl)
		AC_CHECK_LIB(comn, comn_str_to_ascii, WITH_SYBASE_COMN="yes", WITH_SYBASE_COMN="no", -lintl)
		AC_CHECK_LIB(intl, intl_open, WITH_SYBASE_INTL="yes", WITH_SYBASE_INTL="no")
		AC_CHECK_LIB(sybtcl, netp_init_driver_poll, WITH_SYBASE_SYBTCL="yes", WITH_SYBASE_SYBTCL="no", -lcomn -linsck -lintl)
		AC_CHECK_LIB(cs, cs_time, WITH_SYBASE_CS="yes", WITH_SYBASE_CS="no", -lcomn -lintl)
		AC_CHECK_LIB(insck, bsd_tcp, WITH_SYBASE_INSCK="yes", WITH_SYBASE_INSCK="no", -lcomn -lintl)
		if test "$WITH_SYBASE_CT" = "no" \
		    -o "$WITH_SYBASE_COMN" = "no" \
		    -o "$WITH_SYBASE_INTL" = "no" \
		    -o "$WITH_SYBASE_SYBTCL" = "no" \
		    -o "$WITH_SYBASE_CS" = "no" \
		    -o "$WITH_SYBASE_CS" = "no" \
		    -o "$WITH_SYBASE_INSCK" = "no"; then
		    WITH_SYBASE="no"
		else
		    SYBASE_LIBS="-L$sybase_libdir -lct -lcomn -lintl -lsybtcl -lcs -linsck"
		fi
		LIBS="$temp"
	    fi

	    AC_MSG_CHECKING(if Sybase support can be enabled)
	    if test "$WITH_SYBASE" = "yes"; then
		AC_DEFINE(USE_SYBASE)
		OBJDRIVERMODULES="$OBJDRIVERMODULES objdrv_sybase.so"
		AC_MSG_RESULT(yes)
	    else
		AC_MSG_RESULT(no)
	    fi
	fi

	AC_SUBST(SYBASE_LIBS)
	AC_SUBST(SYBASE_CFLAGS)
    ]
)

dnl Test for the HTTP os driver.
AC_DEFUN(CENTRALLIX_CHECK_HTTP_OS,
    [
	AC_MSG_CHECKING(if HTTP support is desired)

	AC_ARG_ENABLE(http,
	    AC_HELP_STRING([--disable-http],
		[disable HTTP support]
	    ),
	    WITH_HTTP="$enableval", 
	    WITH_HTTP="yes"
	)
 
	if test "$WITH_HTTP" = "yes"; then
	    AC_DEFINE(USE_HTTP)
	    OBJDRIVERMODULES="$OBJDRIVERMODULES objdrv_http.so"
	    AC_MSG_RESULT(yes)
	else
	    AC_MSG_RESULT(no)
	fi
    ]
)


dnl Test for the XML os driver.
AC_DEFUN(CENTRALLIX_CHECK_XML_OS,
    [
	AC_MSG_CHECKING(if xml support is desired)

	AC_ARG_ENABLE(xml,
	    AC_HELP_STRING([--disable-xml],
		[disable xml support]
	    ),
	    WITH_XML="$enableval", 
	    WITH_XML="yes"
	)

	AC_ARG_WITH(libxml,
	    AC_HELP_STRING([--with-libxml=PATH],
		[library path for xml library (default is /usr/lib)]
	    ),
	    libxml_libdir="$withval",
	    libxml_libdir="`xml2-config --libs | cut -f1 | cut -b 3-`"
	)

	AC_ARG_WITH(libxml-inc,
	    AC_HELP_STRING([--with-libxml-inc=PATH],
		[include path for libxml headers (default is /usr/include)]
	    ),
	    libxml_incdir="$withval",
	    libxml_incdir="`xml2-config --cflags | cut -f1 | cut -b 3-`"
	)
 
	if test "$WITH_XML" = "no"; then
	    AC_MSG_RESULT(no)
	else
	    AC_MSG_RESULT(yes)
	    temp=$CPPFLAGS
	    CPPFLAGS="$CPPFLAGS -I$libxml_incdir"
	    tempC=$CFLAGS
	    CFLAGS="$CFLAGS -I$libxml_incdir"
	    AC_CHECK_HEADER(libxml/parser.h, 
		WITH_XML="yes",
		WITH_XML="no"
	    )
	    CPPFLAGS="$temp"
	    CFLAGS="$tempC"

	    if test "$WITH_XML" = "yes"; then
		XML_CFLAGS="-I$libxml_incdir"
		temp=$LIBS
		LIBS="$LIBS -L$libxml_libdir"
		AC_CHECK_LIB(xml2, xmlParseFile, WITH_XML_XML="yes", WITH_XML_XML="no", -lxml2)
		if test "$WITH_XML_XML" = "no"; then
		    WITH_XML="no"
		else
		    XML_LIBS="-L$libxml_libdir -lxml2"
		fi
		LIBS="$temp"
	    fi
	fi

	AC_MSG_CHECKING(if XML support can be enabled)
	if test "$WITH_XML" = "yes"; then
	    AC_DEFINE(USE_XML)
	    OBJDRIVERMODULES="$OBJDRIVERMODULES objdrv_xml.so"
	    AC_MSG_RESULT(yes)
	else
	    AC_MSG_RESULT(no)
	fi

	AC_SUBST(XML_CFLAGS)
	AC_SUBST(XML_LIBS)
    ]
)
