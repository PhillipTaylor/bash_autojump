#!/bin/bash

#
#collect the files:
#	ajump.def  autojump.c  autojump.h  common.c.patch  Makefile.in.patch
#into an easy-to-run bash source tree patch script.
#

FILES="ajump.def  autojump.c  autojump.h  common.c.patch  Makefile.in.patch"

OUT=bash_patch.sh
[ -z $1 ] || OUT=$1

cat > ${OUT} <<EOF
#!/bin/bash

#autogen'd script to patch bash source tree
#with the wicked-cool autojump feature.
#
#usage:

EOF

echo "#  "$(basename $OUT)" [<path_to_bash_src_tree>]" >> ${OUT}
cat >> ${OUT} <<EOF

TARGDIR="."
SRCDIR=\$(pwd)

if [ ! -z "\$1" ]; then
   TARGDIR=\$1
fi
if [ -d "\$TARGDIR" ]; then
   if [ "\$(basename \$TARGDIR)" != "builtins" ]; then
      if [ -d "\$TARGDIR/builtins" ]; then
         TARGDIR=\$TARGDIR/builtins
      fi
   fi
fi

if [ "\$(basename \$TARGDIR)" != "builtins" ]; then
   echo "Error: Cannot locate bash src tree."
   exit 1
fi

TARFILE=\${SRCDIR}/ajump_tar\$\$.tgz

##read through to find where the tar file contents begin
line_no=1
while [ 1 ]; do
   read DATA
   [ \$? -ne 0 ] && break

   line_no=\$((line_no + 1))
   if [ "\$DATA" = "__CONTENTS" ]; then
      break
   fi
done < \${0}

echo \${line_no}
tail -n +\${line_no} \${0} > \${TARFILE}

cd \${TARGDIR}

tar xvzf \${TARFILE}
rm -f \${TARFILE}
patch -p0 < Makefile.in.patch
patch -p0 < common.c.patch

rm -f Makefile.in.patch
rm -f common.c.patch

exit 0

__CONTENTS
EOF

tar cvz $FILES >> ${OUT}
