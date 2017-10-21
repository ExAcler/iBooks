/*

iBooks Pro C
Sorting algorithms source file

(c)2013 - 2017 Xhorizon, Some rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdlib.h>

void myswap(void *e1, void *e2, int size) {
    void* sp = (void*) malloc(size);
    memcpy(sp, e1, size);
    memcpy(e1, e2, size);
    memcpy(e2, sp, size);
    free(sp);
}
 
void qsort(void* base, int n, int size, int (*cmp)(const void*, const void*)) {
    int i, j;
 
    if (n <= 1) {return;}

    i = 0; j = n;
    while (1) {
        do {
            ++i;
        } while(cmp(base + size*i, base) < 0 && i < n);
        do {
            --j;
        } while(cmp(base + size*j, base) > 0);
        if (i > j) break;
        myswap(base + size*i, base + size*j, size);
    }
    if (j != 0) {
        myswap(base, base+size*j, size);
    }
    if (j > 0) {
        qsort(base, j, size, cmp);
    }
    qsort(base + size*(j+1), n - 1 - j, size, cmp);
}