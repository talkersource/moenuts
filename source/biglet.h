/* Big letter array map - for greet() */
int biglet[26][5][5]={
	{{0,1,1,1,0},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1}},
	{{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0}},
	{{0,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{0,1,1,1,1}},
	{{1,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,0}},
	{{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,0},{1,1,1,1,1}},
	{{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0}},
	{{0,1,1,1,0},{1,0,0,0,0},{1,0,1,1,0},{1,0,0,0,1},{0,1,1,1,0}},
	{{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1}},
	{{0,1,1,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,1,1,1,0}},
	{{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}},
	{{1,0,0,0,1},{1,0,0,1,0},{1,0,1,0,0},{1,0,0,1,0},{1,0,0,0,1}},
	{{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,1}},
	{{1,0,0,0,1},{1,1,0,1,1},{1,0,1,0,1},{1,0,0,0,1},{1,0,0,0,1}},
	{{1,0,0,0,1},{1,1,0,0,1},{1,0,1,0,1},{1,0,0,1,1},{1,0,0,0,1}},
	{{0,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}},
	{{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0}},
	{{0,1,1,1,0},{1,0,0,0,1},{1,0,1,0,1},{1,0,0,1,1},{0,1,1,1,0}},
	{{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,1,0},{1,0,0,0,1}},
	{{0,1,1,1,1},{1,0,0,0,0},{0,1,1,1,0},{0,0,0,0,1},{1,1,1,1,0}},
	{{1,1,1,1,1},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}},
	{{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1}},
	{{1,0,0,0,1},{1,0,0,0,1},{0,1,0,1,0},{0,1,0,1,0},{0,0,1,0,0}},
	{{1,0,0,0,1},{1,0,0,0,1},{1,0,1,0,1},{1,1,0,1,1},{1,0,0,0,1}},
	{{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,1,0,1,0},{1,0,0,0,1}},
	{{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}},
	{{1,1,1,1,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{1,1,1,1,1}}
	};

/* Symbol array map - for greet() */
int bigsym[32][5][5]={
	{{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,1,0,0}},
	{{0,1,0,1,0},{0,1,0,1,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,1,0,1,0},{1,1,1,1,1},{0,1,0,1,0},{1,1,1,1,1},{0,1,0,1,0}},
	{{0,1,1,1,1},{1,0,1,0,0},{0,1,1,1,0},{0,0,1,0,1},{1,1,1,1,0}},
	{{1,1,0,0,1},{1,1,0,1,0},{0,0,1,0,0},{0,1,0,1,1},{1,0,0,1,1}},
	{{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,1,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,1,1,0},{0,1,0,0,0},{0,1,0,0,0},{0,1,0,0,0},{0,0,1,1,0}},
	{{0,1,1,0,0},{0,0,0,1,0},{0,0,0,1,0},{0,0,0,1,0},{0,1,1,0,0}},
	{{1,0,1,0,1},{0,1,1,1,0},{1,1,1,1,1},{0,1,1,1,0},{1,0,1,0,1}},
	{{0,0,1,0,0},{0,0,1,0,0},{1,1,1,1,1},{0,0,1,0,0},{0,0,1,0,0}},
	{{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,1,0,0,0},{1,1,0,0,0}},
	{{0,0,0,0,0},{0,0,0,0,0},{1,1,1,1,1},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{1,1,0,0,0},{1,1,0,0,0}},
	{{0,0,0,0,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{1,0,0,0,0}},
	{{0,1,1,1,0},{1,0,0,1,1},{1,0,1,0,1},{1,1,0,0,1},{0,1,1,1,0}},
	{{0,0,1,0,0},{0,1,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,1,1,1,0}},
	{{1,1,1,1,0},{0,0,0,0,1},{0,1,1,1,0},{1,0,0,0,0},{1,1,1,1,1}},
	{{1,1,1,1,0},{0,0,0,0,1},{0,1,1,1,0},{0,0,0,0,1},{1,1,1,1,0}},
	{{0,0,1,1,0},{0,1,0,0,0},{1,0,0,1,0},{1,1,1,1,1},{0,0,0,1,0}},
	{{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{0,0,0,0,1},{1,1,1,1,0}},
	{{0,1,1,1,0},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,1},{0,1,1,1,0}},
	{{1,1,1,1,1},{0,0,0,0,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0}},
	{{0,1,1,1,0},{1,0,0,0,1},{0,1,1,1,0},{1,0,0,0,1},{0,1,1,1,0}},
	{{1,1,1,1,1},{1,0,0,0,1},{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1}},
	{{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,1,0,0,0}},
	{{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{0,0,1,0,0},{0,0,0,1,0}},
	{{0,0,0,0,0},{1,1,1,1,1},{0,0,0,0,0},{1,1,1,1,1},{0,0,0,0,0}},
	{{0,1,0,0,0},{0,0,1,0,0},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0}},
	{{0,1,1,1,1},{0,0,0,0,1},{0,0,1,1,1},{0,0,0,0,0},{0,0,1,0,0}},
	{{0,1,0,0,0},{1,0,1,1,1},{1,0,1,0,1},{1,0,1,1,1},{0,1,1,1,0}}
	};
