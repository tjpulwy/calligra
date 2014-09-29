#! /bin/sh
cd ../../..
source ../calligra_xgettext.sh

potfile=kexipostgresqldriver
find_exclude $potfile

LIST="`find . \( $EXCLUDE \) -prune -o \( -name \*.ui \) -type f -print \
| egrep "kexidb/drivers/pqxx/|migration/pqxx/" \
| grep -v -e '/\.'`"
if test -n "$LIST"; then
    $EXTRACTRC $LIST >> rc.cpp
fi

# Exclude files containing "#warning noi18n"
LIST=`find . \( $EXCLUDE \) -prune -o \( -name \*.h -o -name \*.cpp -o -name \*.cc -o -name \*.hxx -o -name \*.cxx \) -type f -print \
| egrep "kexidb/drivers/pqxx/|migration/pqxx/" \
| sort | while read f ; do \
    if ! grep -q '^#warning noi18n ' $f ; then echo $f; fi \
done \
`
calligra_xgettext $potfile.pot $LIST
rm -f rc.cpp