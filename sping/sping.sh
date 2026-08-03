#!/usr/bin/env bash

hosts=(
    1.1.1.1
    9.9.9.9
    discord.com
    instagram.com
    google.com
    208.67.222.222
)

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

for i in "${!hosts[@]}"; do
(
    ping -c1 -W1 "${hosts[i]}" 2>/dev/null |
    awk -F'time=' '/time=/{split($2,a," "); print a[1]}'
) >"$tmp/$i" &
done

wait

awk '
{
    if ($1 != "") {
        sum += $1
        n++
    }
}
END {
    if (n)
        printf "%.0f\n", sum/n
    else
        print "N/A"
}
' "$tmp"/*
