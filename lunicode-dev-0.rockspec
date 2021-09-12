rockspec_format = "3.0"

package = "lunicode"
version = "dev-0"
source = {
   url = "git+https://github.com/Mehgugs/lunicode.git"
}
description = {
   homepage = "https://github.com/Mehgugs/lunicode",
   license = "MIT"
}

external_dependencies = {
   UTF8PROC = {
      header = "utf8proc.h"
   }
}

build = {
   type = "builtin",
   modules = {
      lunicode = {
         sources = {'src/lunicode.c'},
         incdirs = {'$(UTF8PROC_INCDIR)'},
         libraries = {
            'utf8proc'
         }
      }
   },
   platforms = {
      windows = {
         modules = {
            lunicode = {
               sources = {'src/lunicode.c'},
               incdirs = {'$(UTF8PROC_INCDIR)'},
               libraries = {
                  '$(UTF8PROC_LIBDIR)/utf8proc'
               }
            }
         }
      }
   }
}
