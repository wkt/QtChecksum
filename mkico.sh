#!/bin/bash

infile=$1
if test -z "${infile}";then
	exit 1
fi

outfile=$2
test -z "${outfile}" && {
	outfile="icon.ico"
	i=1
	while test -f "$outfile";
	do
		outfile="icon_$i.ico"
		let i=$i+1
	done
}

tmpdir="/tmp/${USER}_tmp.icons"
rm -rf "${tmpdir}"
mkdir -p "${tmpdir}"


pngs=""
for s in 16 32 64 128 256
do
    convert $infile -resize ${s}x${s} ${tmpdir}/${s}.png
    pngs="$pngs ${tmpdir}/${s}.png"
done

icotool -c -o $outfile $pngs

#rm -rf "${tmpdir}"
