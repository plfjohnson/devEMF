#!/usr/bin/perl -w

# Use Adobe glyph name to unicode mapping found at
#   https://github.com/adobe-type-tools/agl-aglfn
# to convert the 14 core Adobe pdf afm files from adobe encoding to
# unicode encoding found linked off
#   http://www.adobe.com/devnet/font.html
# and specifically
#   http://wwwimages.adobe.com/content/dam/Adobe/en/devnet/font/pdfs/Core14_AFMs.tar

use strict;

my %name2unicode;
my @lookupTables = ("glyphlist.txt", "zapfdingbats.txt");
foreach my $fn (@lookupTables) {
    open INFILE, $fn;
    while (<INFILE>) {
        next if /^#/;
        chomp;
        my ($name, $unicode) = split /;/;
        $name2unicode{$name} = $unicode;
    }
    close INFILE;
}

foreach my $fn (@ARGV) {
    my ($outfn) = $fn =~ /(.+)\.afm/;
    $outfn .= '-ucs.afm';
    open OUTFILE, ">$outfn";
    open INFILE, $fn;
    while (<INFILE>) {
        if (/^C /) {
            my ($name) = /N (.+?) ;/;
            if (!exists($name2unicode{$name})) {
                warn("glyph '$name' not found in table!");
            } else {
                s/^C (.+?) ;/C $name2unicode{$name} ;/;
            }
        } elsif (/^EncodingScheme/) {
            $_ = "EncodingScheme UCS-2\n";
        }
        if ($. == 2) {
            print OUTFILE "Comment Modified by Philip Johnson in 2016 to use Unicode rather than Adobe encoding (converting from glyph names to unicode using files at https://github.com/adobe-type-tools/agl-aglfn).\n"
        }
        print OUTFILE $_;
    }
    close INFILE;
    close OUTFILE;
}
