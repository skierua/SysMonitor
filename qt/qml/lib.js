.pragma library

/**
* human presentation of memory size
* param(v) unsigned long long - memory size
*/
function humanMem(v, dec = 1) {
    let i = 1;
    let base = 1024;
    let pow = base;
    let tmp = v;
    for ( ; tmp > pow * base && i < 4; ++i, pow *= base) {}
    if (i === 2) return ((tmp/pow).toFixed(dec) + " MB");
    else if (i === 1) return ((tmp/pow).toFixed(dec) + " kB");
    else if (i === 3) return ((tmp/pow).toFixed(dec) + " GB");
    else if (i === 0) return ((tmp/pow).toFixed(dec) + " B");
    else return String(v);
    // return "";
}
