[= AutoGen5 Template =][=

  (define (protect-text t)
    (string-substitute t
       '("["     "]"     "$"     "#"     )
       '("@<:@"  "@:>@"  "@S|@"  "@%:@"  ))
  )

=][=

DEFINE preamble

=][=

  (define test-name (get "name"))
  (if (= (string-length test-name) 0)
      (set! test-name "test_name")
      (string->c-name! test-name) )

  (define up-name     (string-upcase    test-name))
  (define down-name   (string-downcase  test-name))
  (define group-id    (string-downcase! (get "group")))
  (if (= (string-length group-id) 0)
      (set! group-id "ac")
      (string->c-name! group-id)  )
  (define group-pfx   (string-append    group-id "_"))
  (define mac-name    (string-upcase!   (string-append group-pfx
                      (get "type") "_" up-name)))
  (define sub-name    (string-upcase!   (string-append group-pfx down-name)))
  (define cv-name     (string-downcase! (string-append group-pfx "cv_"
                      (get "type") "_" down-name)))

=][=

ENDDEF   preamble

=][=

DEFINE  emit-macro      =][=

  CASE    type   =][=
  ~~  compile|run|link|test   =][=
      (define bad-define-name   "NEED_%s")
      (define good-define-name  "HAVE_%s")     =][=
  ~~  enable|disable     =][=
      (define bad-define-name   "%s_DISABLED")
      (define good-define-name  "%s_ENABLED")  =][=
  ~~  with|without       =][=
      (define bad-define-name   "WITHOUT_%s")
      (define good-define-name  "WITH_%s")     =][=
  ESAC  =][=

(dne "dnl " "dnl ") =]
dnl
dnl @synopsis  [=(. mac-name)=]
dnl
dnl @success-result[=
  IF (< (count "action") (count "action.no")) =]:  there is no output[=
  ELSE         =]
dnl[=
    FOR action =][=
      IF (not (exist? "no"))  =][=
        CASE  act-type  =][=
        ==  define      =]
dnl   * [=(sprintf good-define-name up-name)
        =] is #defined as [=?% act-text "%s" "1"=][=
        ==  subst       =]
dnl   * @[=(.  sub-name)=]@ is replaced by [=act-text=][=
        ==  script      =]
dnl   * a short script is run[=
        ESAC   =][=
      ENDIF    =][=
    ENDFOR     =][=
  ENDIF        =]
dnl
dnl @failure-result[=
  IF (= (count "action.no") 0) =]:  there is no output[=
  ELSE         =]
dnl[=
    FOR action =][=
      IF (exist? "no")  =][=
        CASE  act-type  =][=
        ==  define      =]
dnl   * [=(sprintf bad-define-name up-name)
       =] is #defined as [=?% act-text "%s" "1"=][=
        ==  subst       =]
dnl   * @[=(. sub-name)=]@ is replaced by [=act-text=][=
        ==  script      =]
dnl   * a short script is run[=
        ESAC   =][=
      ENDIF    =][=
    ENDFOR     =][=
  ENDIF        =][=

  IF   (define doc-text (get "doc"))
     (> (string-length doc-text) 0) =]
dnl
dnl @description
[=(prefix "dnl   " doc-text) =][=
  ENDIF =][=

IF  (exist? "author") =]
dnl
dnl @version "[=

 (strftime "%d-%B-%Y at %H:%M"
           (localtime (current-time)) ) =]"
dnl
dnl @author [=author=][=

ENDIF     =]
dnl
AC_DEFUN([[=
  (define fcn-name (string-append "try-" (get "type")))
  (define test-text (get "code"))
  (if (> (string-length test-text) 0)
      (set! test-text (string-append "[" (protect-text test-text) "]"  )))

  mac-name  =]],[[=

  IF (ag-function? fcn-name) =][=
    INVOKE (. fcn-name) =][=

  ELSE   =]

ERROR:  invalid conftest function:   [= (. fcn-name) =][=

  ENDIF  =]
]) # end of AC_DEFUN of [=(. mac-name)=]
[=

ENDDEF  emit-macro   =][=

# # # # # # # # # # # C-Feature # # # # # # # # # #

Stash the result of a C/C++ feature test =][=

DEFINE start-feat-test =]
  AC_MSG_CHECKING([whether [=(protect-text (get "check"))=]])[=
ENDDEF start-feat-test =][=

DEFINE  end-feat-test  =][=

  (. pop-language)     =]]) # end of CACHE_VAL
  AC_MSG_RESULT([${[=(. cv-name)=]}])[=
  emit-results         =][=

ENDDEF  end-feat-test  =][=

# # # # # # # # # # EMIT RESULTS # # # # # # # # # # =][=

DEFINE  emit-results     =][=

  (define good-subst 0 )
  (define bad-subst  0 )
  (define good-text  "")
  (define bad-text   "")
  (define tmp-text   "") =][=

  FOR     action         =][=

    CASE (set! tmp-text (get "act-text"))
	     (string-append
           (if (exist? "no") "no-" "yes-")
           (get "act-type"))
      =][=

    == yes-define        =][=
      (set! good-text (string-append good-text
            "\n    AC_DEFINE([" (sprintf good-define-name up-name) "],["
			(if (> (string-length tmp-text) 0) tmp-text "1")
            "],\n        [Define this if " (protect-text (get "check")) "])" ))
      =][=

    == yes-subst         =][=
      (set! good-subst 1)
	  (set! good-text (string-append good-text
                   "\n    " sub-name "=" (protect-text (shell-str tmp-text)) ))
      =][=

    == yes-script        =][=
      (set! good-text (string-append good-text "\n    "
            (protect-text tmp-text)))
      =][=

    ==  no-define        =][=
      (set! good-text (string-append good-text
            "\n    AC_DEFINE([" (sprintf bad-define-name up-name) "],["
			(if (> (string-length tmp-text) 0) tmp-text "1")
            "],\n        [Define this if '" (protect-text (get "check"))
                                       "' is not true])" ))
      =][=

    ==  no-subst         =][=
      (set! bad-subst  1)
	  (set! bad-text (string-append bad-text
                   "\n    " sub-name "=" (protect-text (shell-str tmp-text)) ))
      =][=

    ==  no-script        =][=
      (set! bad-text (string-append bad-text "\n    "
            (protect-text tmp-text)))
      =][=

    ESAC                 =][=
  ENDFOR  action         =][=

  (if (> good-subst 0)
      (if (< bad-subst 1)
          (set! bad-text (string-append bad-text "\n    "
                         sub-name "=''" ))  )
      (if (> bad-subst 0)
          (set! good-text (string-append good-text "\n    "
                         sub-name "=''" ))  )
  )

=]

  if test "X${[=(. cv-name)=]}" [=

  IF (> (string-length good-text) 0)

    =]!= Xno
  then[=
    (. good-text) =][=

    IF (> (string-length bad-text) 0) =]
  else[=
    ENDIF         =][=
  ELSE            =]= Xno
  then[=
  ENDIF           =][=
    (. bad-text)  =]
  fi[= (if (> (+ good-subst bad-subst) 0)
           (string-append "\n  AC_SUBST([" sub-name "])" ))

  =][=
ENDDEF  emit-results   =][=

# # # # # # # # # # ENABLEMENT # # # # # # # # # # =][=

DEFINE  emit-enablement

  =]
  AC_ARG_[=arg-name=]([[=(. down-name)=]],
    AC_HELP_STRING([--[=type=]-[=(string-tr test-name "_A-Z" "-a-z")
                   =]], [[=check=]]),
    [[=(. cv-name)=]=${[=(string-downcase! (get "arg-name"))
      =]_[=(string-tr test-name "-A-Z" "_a-z")=]}],
    AC_CACHE_CHECK([whether [=check=]], [=(. cv-name)=],
      [=(. cv-name)=]=[=
         (if (~~ (get "type") "with|enable") "no" "yes") =])
  ) # end of AC_ARG_[=arg-name=][=

  emit-results =][=

ENDDEF  emit-enablement

=][=

# # # # # # # # # SET-LANGUAGE # # # # # # # # =][=

DEFINE  set-language

=]
  AC_CACHE_VAL([[=(. cv-name)=]],[[=
  CASE language  =][=
  ==   ""        =][=(define pop-language "")=][=
  ==   default   =][=(define pop-language "")=][=
  *              =]
  AC_LANG_PUSH([=language=])[=
    (define pop-language (sprintf "
  AC_LANG_POP(%s)" (get "language"))) =][=

  ESAC  =][=

ENDDEF  set-language            =][=

# # # # # # # # # # WITH # # # # # # # # # =][=

DEFINE  try-with                =][=
  emit-enablement
       arg-name  = WITH         =][=
ENDDEF  try-with                =][=

# # # # # # # # # # WITHOUT # # # # # # # # # =][=

DEFINE  try-without             =][=
  (set! cv-name (string-append group-pfx "cv_with_" down-name)) =][=
  emit-enablement
       arg-name  = WITH         =][=
ENDDEF  try-without             =][=

# # # # # # # # # # ENABLE # # # # # # # # # =][=

DEFINE  try-enable              =][=
  emit-enablement
       arg-name  = ENABLE       =][=
ENDDEF  try-enable              =][=

# # # # # # # # # # DISABLE # # # # # # # # # =][=

DEFINE  try-disable             =][=
  (set! cv-name (string-append group-pfx "cv_enable_" down-name)) =][=
  emit-enablement
       arg-name  = ENABLE       =][=
ENDDEF  try-disable             =][=

# # # # # # # # # # TEST # # # # # # # # # =][=

DEFINE  try-test                =][=

  start-feat-test               =]
  AC_CACHE_VAL([[=(. cv-name)=]],[ changequote(<<<,>>>)
    if [= (sub-shell-str (get "code")) =] > /dev/null 2>&1
    then [=(. cv-name)=]=yes
    else [=(. cv-name)=]=no ; fi changequote([,])
  ]) # end of CACHE_VAL
  AC_MSG_RESULT([${[=(. cv-name)=]}])[=
  emit-results         =][=

ENDDEF  try-disable             =][=

# # # # # # # # # # RUN # # # # # # # # # =][=

DEFINE  try-run                 =][=

  start-feat-test               =][=
  set-language                  =]
  AC_TRY_RUN([=
     (. test-text)=],
    [[=(. cv-name)=]=yes],[[=(. cv-name)=]=no],[[=
        (. cv-name)=]=no]
  ) # end of TRY_RUN[=
  end-feat-test                 =][=

ENDDEF  try-run                 =][=

# # # # # # # # # # LINK # # # # # # # # # =][=

DEFINE  try-link                =][=

  start-feat-test               =][=
  set-language                  =]
  AC_TRY_LINK([=
     (. test-text)=],
    [[=(. cv-name)=]=yes],[[=(. cv-name)=]=no]
  ) # end of TRY_LINK[=
  end-feat-test                 =][=

ENDDEF  try-link                =][=

# # # # # # # # # # COMPILE # # # # # # # # # # =][=

DEFINE  try-compile             =][=

  start-feat-test               =][=
  set-language                  =]
  AC_TRY_COMPILE([= % includes "[%s]" =],[=
     (. test-text)=],
    [[=(. cv-name)=]=yes],[[=(. cv-name)=]=no]
  ) # end of TRY_COMPILE[=
  end-feat-test                 =][=

ENDDEF  try-compile             =]
