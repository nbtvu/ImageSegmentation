#pragma once

// quicksort function
void quickSort(Edge *edges, int l, int r) {
	if (l >= r) {
		return;
	}
	int i = l;
	int j = r;
	double pivot = edges[(l + r) / 2].w;
	do {
		while (edges[i].w < pivot) {
			i++;
		}
		while (edges[j].w > pivot) {
			j--;
		}
		if (i <= j) {
			Edge tmp = edges[i];
			edges[i] = edges[j];
			edges[j] = tmp;
			i++;
			j--;
		}
	} while (i <= j);
	quickSort(edges, l, j);
	quickSort(edges, i, r);
}
