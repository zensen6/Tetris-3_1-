#include "tetris.h"

static struct sigaction act, oact;

int main(){
	int exit=0;
	
	initscr();
	noecho();
	keypad(stdscr, TRUE);	

	srand((unsigned int)time(NULL));
	createRankList();
	//////
	recRoot = (RecNode *)malloc(sizeof(RecNode));
	recRoot->lv = -1;
	recRoot->score = 0;
	recRoot->f = field;
	constructRecTree(recRoot);
	//////


	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_RANK: rank(); break;
		case MENU_R_PLAY: recommendedPlay(); break;
		case MENU_EXIT: exit=1; break;
		default: break;
		}
	}

	endwin();
	system("clear");
	writeRankFile();
	return 0;
}

void InitTetris(){
	int i,j;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;

	int k=0;
	while(k<3){
		
		nextBlock[k] = rand()%7;
		k++;
		
	}
	recommend();
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;

	DrawOutline();
	DrawField();
	DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline(){	
	int i,j;
	
	DrawBox(0,0,HEIGHT,WIDTH);

	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);
	DrawBox(8,WIDTH+10,4,8);

	move(19,WIDTH+10);
	printw("SCORE");
	DrawBox(20,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score){
	
	///
	move(21,WIDTH+11);
	printw("%8d",score);
	///
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	int k=1;
	while(k<3){
		for( i = 0; i < 4; i++ ){
			move(4+i+(k-1)*5,WIDTH+13);
			for( j = 0; j < 4; j++ ){
				if( block[nextBlock[k]][0][i][j] == 1 ){
					attron(A_REVERSE);
					printw(" ");
					attroff(A_REVERSE);
				}
				else printw(" ");
			}
		}
		k++;	
	}

}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	
	int i, j;
	for(i = 0; i < 4; i++){ 
		for(j = 0; j < 4; j++){
			if(block[currentBlock][blockRotate][i][j] == 1) {
				if(f[blockY+i][blockX+j]) return 0;
				if(blockY + i >= HEIGHT) return 0;
				if(blockX + j < 0) return 0;
				if(blockX + j >= WIDTH) return 0;
			}
		}
	}
	return 1;
	
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	/*
	int y = blockY;
	int x = blockX;
	int br = blockRotate;
	int curb = currentBlock;
	
	switch(command){
		
		case KEY_LEFT:
			x++;
			break;
		case KEY_UP:
			br = (br+3)%4;
			break;
		case KEY_RIGHT:
			x--;	
			break;
		case KEY_DOWN:
			y--;
			break;
	}
	
	
	for(int i=0; i<4; i++){
		for(int j=0; j<4; j++){
			
			if(block[curb][br][i][j]==1 && i+y>=0){
				
				move(i+y+1,j+x+1);
				printw(".");	
				
			}
		}	
	}
	
	DrawField();
	DrawBlockWithFeatures(blockY,blockX,currentBlock,blockRotate);
	///
	move(HEIGHT,WIDTH+10);
	
	///
	*/	
	int i, j;
	int blk = currentBlock, rot = blockRotate, y = blockY, x = blockX;
	int oldShadowY;
	switch (command) {
	case KEY_UP:
		rot = (rot + 3) % 4;
		break;
	case KEY_DOWN:
		y--;
		break;
	case KEY_LEFT:
		x++;
		break;
	case KEY_RIGHT:
		x--;
		break;
	}
	oldShadowY = y;
	while (CheckToMove(f, blk, rot, ++oldShadowY, x));
	--oldShadowY;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++) {
			if (block[blk][rot][i][j] == 1) {
				if (i + y >= 0) { 
					move(i + y + 1, j + x + 1);
					printw(".");
				}
				if (i + oldShadowY >= 0) { 
					move(i + oldShadowY + 1, j + x + 1);
					printw(".");
				}
			}
		}

	DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate);
	move(HEIGHT, WIDTH + 10);
}

void BlockDown(int sig){
	/*
	if(CheckToMove(field, nextBlock[0], blockRotate, blockY+1, blockX)){
		blockY++;
	}

	else if(blockY == -1) gameOver = 1;

	else{
		
		score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
		score += DeleteLine(field);
		
		for(int i=0;i<=2;i++){
			
			if(i==2){
				nextBlock[i] = rand()%7;
			}
			else{
				nextBlock[i] = nextBlock[i+1];
			}
		}
		
		blockRotate=0;
		blockY=-1;
		blockX=(WIDTH/2)-2;
		
		recommend();
		DrawNextBlock(nextBlock);
		
		DrawField();
		DrawNextBlock(nextBlock);
		
	}
	PrintScore(score);
	DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
	timed_out=0;
	*/
	
	int drawFlag = 0;
	int i;
	if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX))) {
		blockY++;
		DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
	}
	else {
		if (blockY == -1) gameOver = 1;
		score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
		score += DeleteLine(field);
		blockY = -1;
		blockX = (WIDTH / 2) - 2;
		blockRotate = 0;
		i=0;
		//
		while(i<=2) {
			if(i<2){
					
				nextBlock[i] = nextBlock[i + 1];
			}
			else{
				
				nextBlock[i] = rand() % 7;		
			}
			i++;
		}
		
		recommend();
		DrawNextBlock(nextBlock);
		PrintScore(score);
		DrawField();
		DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
	}
	timed_out = 0;
	
}

int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	
	int alphasc = 0;
	for(int i=0; i<4; i++) {
		for(int j=0; j<4; j++) {
			if(block[currentBlock][blockRotate][i][j] == 1) {
				int y = blockY+i;
				int x = blockX+j;
				f[y][x] = 1;
				if(y>=HEIGHT-1 || f[y+1][x] == 1)
					alphasc++;
					
			}
		}
	}
	return alphasc*10;
}

int DeleteLine(char f[HEIGHT][WIDTH]){

	int cnt=0;
	int isfull=1;
	for(int i=0; i<HEIGHT; i++){
		isfull = 1;
		for(int j=0; j<WIDTH; j++){
			
			if(f[i][j]==0){
				
				isfull=0;
				break;
				
			}
			
			
		}
		if(isfull){
			
			cnt++;	
			for(int k=0;k<i;k++){
			
				for(int j = 0; j < WIDTH; j++){
					
					f[i-k][j] = f[i-k-1][j];
					
				}
				
			}
			
		
		}
		
	}
	return cnt*cnt*100;

}

void DrawShadow(int y, int x, int blockID,int blockRotate){
	
	int blockY=y;
	while(CheckToMove(field,blockID,blockRotate,blockY+1,x)==1)
		blockY++;
	DrawBlock(blockY,x,blockID,blockRotate,'/');	
	
}

void insert(char nametmp[20], int scoretmp) { //링크드 리스트 핵심... 실습은 추가만 하는거라 insert만 구현합니다.
	
	Node *tmp = head;
	Node *prev = head;
	Node *new0 = (Node*)malloc(sizeof(Node));
	//memset(new0->name, 0 ,sizeof(new0->name));
	/* Memory Allocation.*/
	//새로운 노드 new를 만든다.
	new0->score=scoretmp;
	new0->link=NULL;
	strcpy(new0->name,nametmp);
	
	//////////////////////////////

	
	if (tmp == NULL)// head가 비어있을 때
		head = new0;
		
	else {
		if (tmp->score <= new0->score) {	//맨 앞에 넣어야 할 때 tmp->link=null
			new0->link = head;
			head = new0;
		}
		else {
		
			while(tmp->link!=NULL){
				if(tmp->link->score <= new0->score){
					
					new0->link = tmp->link;
					tmp->link = new0;
					return;	
					
				}
				tmp = tmp->link;	
				
			}
			
			tmp->link = new0;
			
		}
		
	}
	
	return;
}
void createRankList(){
	
	FILE* fp;
	int num, i, scoretmp;
	char str[51], nametmp[NAMELEN];
	head = NULL;
	
	if((fp = fopen("rank.txt","r")) == NULL){
		return;
	}
	
	fscanf(fp,"%d",&num);
	user_num = num;
	fgetc(fp);
	for(i=0; i<user_num; i++){
		
		fgets(str,50,fp);
		sscanf(str, "%s %d", nametmp, &scoretmp);
		insert(nametmp, scoretmp);
	}
	
	fclose(fp);
}


void rank(){
	int from=1;
	int to;
	int loop;
	int i;
	char name[NAMELEN];
	int del;
	Node* tmp;
	clear();
	move(0,0);
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");
	switch(wgetch(stdscr)){
		case '1':
			createRankList();
			to = user_num;
			echo();
			printw("X: ");
			scanw("%d",&from);
			printw("Y: ");
			scanw("%d",&to);
			noecho();
			printw("       name       |   score   \n");
			printw("------------------------------\n");

			// 조건문 삽입 - 실습
			if (from < 0 || to < 0) {
				printw("search failure: no rank in the list\n");
				break;
			}
			else if (from > to) {
				printw("search failure: no rank in the list\n");
				break;
			}
			else if (from == 0 && to == 0) {
				tmp = head;
				while (tmp != NULL) {
					printw("%10s\t\t%8d\n", tmp->name, tmp->score);
					tmp = tmp->link;
				}
			}
			else{
				
				tmp = head;
				loop = to - from +1;
				while(from >1 && tmp!=NULL){
					
					tmp = tmp->link;
					from--;
					
				}
				
				for(i=0; i <loop && tmp!=NULL; i++){
					
					printw("%10s\t\t%8d\n", tmp->name, tmp->score);
					tmp = tmp->link;
				}

			}
			break;
		case '2':
			
			printw("input the name: ");
			echo();
			getstr(name);
			noecho();
			printw("       name       |   score   \n");
			printw("------------------------------\n");

			// 조건문 삽입 - 과제
			
			
			int flag=0;
			tmp=head;
			while(1){
				if(strcmp(tmp->name,name)==0){
					flag=1;
					printw("%-14s| %-16d\n",tmp->name,tmp->score);
				}
				if(tmp->link==NULL) break;
				tmp=tmp->link;
			}
			if(flag==0)
				printw("\nsearch failure: no name in the list\n");
			
			
		//	printw("\nsearch failure: no name in the list\n");
		//	printw("\nresult: the rank deleted\n");

			break;
		case '3':
			printw("input the rank: ");
			echo();
			scanw("%d",&del);
			noecho();
			// 조건문 삽입 - 과제

			
			if(del>user_num || del<=0){
				printw("\nsearch failure: the rank not in the list\n");
			}
			else{
					
				DeleteRank(del);
				printw("\nresult: the rank deleted\n");
				writeRankFile();
					
			}
			
			//printw("\nsearch failure: the rank not in the list\n");
			//printw("\nresult: the rank deleted\n");

			break;
		default: break;
	}
	getch();
	// user code
}

void writeRankFile(){

	Node* tmp;
	
	FILE* fp;
	int i=0;
	if((fp = fopen("rank.txt","r")) != NULL){
		
		fscanf(fp, "%d", &i);
		
	}
	//////////// del from
	else{
		
		return;
		
	}
	
	if( i== user_num ){
		
		return;
		
	}
	
	///////////del to
	
	else {
		
		fclose(fp);
		tmp = head;
		fp = fopen("rank.txt", "w");
		fprintf(fp,"%d\n",user_num);
	
		while(tmp!= NULL){
			
			fprintf(fp,"%s %d\n",tmp->name,tmp->score);
			tmp = tmp->link;
		
		}
		
	}
	

	fclose(fp);
	
}

void newRank(int score){

	char nameTemp[51];
	int i;
	int flag = 0;
	Node *tmp;
	Node *tmp2;
	Node *new0;
	
	clear();
	printw("your name: ");
	echo();
	getstr(nameTemp);
	noecho();
	insert(nameTemp,score);
	//
	user_num++;
	writeRankFile();
	//작성 새로운 정보를 링크드리스트에 집어넣고
	//writeRankFile 호출

}

void DeleteRank(int delnum){
	
	Node* del = head;
	if(delnum==1){
		
		head = head->link;
		free(del);
		user_num--;
		
	}
	else{
		
		Node* prev = head;
		int c=1;
		while(c<delnum){
			
			prev = del;
			del = del->link;
			c++;
			
		}
		prev->link = del->link;
		free(del);
		user_num--;
		
	}
	
	
}
////////////////////////////////////////////

void DrawRecommend() {
	if (CheckToMove(field, nextBlock[0], recommendR, recommendY, recommendX) == 1)
		DrawBlock(recommendY, recommendX, nextBlock[0], recommendR, 'R');//?꾩뿭蹂?섏엫.
}

void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate){
	
	DrawRecommend();
	DrawShadow(y,x,blockID,blockRotate);
	DrawBlock(y,x,blockID,blockRotate,' ');
}

int recommend(){
	//////////////////////////
	int ret;

	
	ret = modified_recommend(recRoot);


	return ret;
	/////////////////////////
}

void constructRecTree(RecNode *root) {
	int i, h;
	RecNode **c = root->c;
	for (i = 0; i < CHILDREN_MAX; ++i) {
		c[i] = (RecNode *)malloc(sizeof(RecNode));
		c[i]->lv = root->lv + 1;
		c[i]->f = (char(*)[WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);
		if (c[i]->lv < VISIBLE_BLOCKS) {
			constructRecTree(c[i]);
		}
	}
}

void destructRecTree(RecNode *root) {
	int i, h;
	RecNode **c = root->c;
	for (i = 0; i < CHILDREN_MAX; ++i) {
		if (c[i]->lv < VISIBLE_BLOCKS) {
			destructRecTree(c[i]);
		}
		free(c[i]->f);
		free(c[i]);
	}
}

int evalState(int lv, char f[HEIGHT][WIDTH], int r, int y, int x) {
	return AddBlockToField(f, nextBlock[lv], r, y, x) + DeleteLine(f);

}

int modified_recommend(RecNode *root) {
	int r, x, y, rBoundary, lBoundary;
	int h, w;
	int eval;
	int max = 0;
	int solR, solY, solX;
	int recommended = 0;
	int i = 0;
	int lv = root->lv + 1;
	///
	int minimum[1000] = {0,};
	int copy[1000] = {0,};
	int min1 = 10000;
	int min2 = 10000;
	///
	RecNode **c = root->c;
	
	////////////////
	// 트리를 탐색하면서 점수를 계산해서 가장 큰 점수가 되는 rotation, x좌표, y좌표를 구하는 겁니다. 
	// 재귀함수로 호출하면서 DFS 방식 
	//리턴값은 최고점수 
	////////////////

	for(r=0;r<NUM_OF_ROTATE;++r){ 
		lBoundary = 3;
		for (h = 0; h<BLOCK_HEIGHT; ++h) {
			for (w = 0; w<BLOCK_WIDTH; ++w) {
				if (block[nextBlock[lv]][r][h][w]) {
				
					break;
					
				}

			}	

			if (w<lBoundary) {
				lBoundary = w;
			}

		}

		lBoundary = 0 - lBoundary;
		rBoundary = 0;

		for (h = 0; h<BLOCK_HEIGHT; ++h) {
			for (w = BLOCK_WIDTH - 1; w >= 0; --w) {
				if (block[nextBlock[lv]][r][h][w]) {
					break;
				}
			}

			if (w>rBoundary) {
				rBoundary = w;
			}

		}

		rBoundary = WIDTH - 1 - rBoundary;
		
		for (x = lBoundary; x <= rBoundary; ++x, ++i) {
			for (h = 0; h<HEIGHT; ++h) {
				for (w = 0; w<WIDTH; ++w) {
				
					c[i]->f[h][w] = root->f[h][w];
				
				}
			}

			y = 0;
			if (CheckToMove(c[i]->f, nextBlock[lv], r, y, x)) {

				while (CheckToMove(c[i]->f, nextBlock[lv], r, ++y, x));
				--y;

			}

			else { 
				continue;
			}

			
			c[i]->score = root->score + evalState(lv, c[i]->f, r, y, x);
			/////
			/*
			if(lv==0){
				
				minimum[i] = c[i]->score;
				copy[i] = c[i]->score;	
				
			}
			*/
			/////
			/////////////////////////////del
			
			if (lv < VISIBLE_BLOCKS - 1) {
				if(c[i]->score > 10){
					
					eval = modified_recommend(c[i]);
						
				}
				else if(lv==0 && c[i]->score<=10){
					
					continue;
					
				}
				
			}
			else {
				eval = c[i]->score;
			}
			if (max<eval) {
				recommended = 1;
				max = eval;
				solR = r;
				solY = y;
				solX = x;
			}	
				
			
			/////////////////////del
			
		}

	}
	
	if (lv == 0 && recommended) {
		recommendR = solR;
		recommendY = solY;
		recommendX = solX;
	}

	return max;

}

///////////////////////////

void updateField(int sig){
	int i;
	if(!CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX)) gameOver=1;
	else{
		score+=AddBlockToField(field,nextBlock[0],recommendR,recommendY,recommendX);
		score+=DeleteLine(field);
		blockY=-1;blockX=(WIDTH/2)-2;blockRotate=0;
		for(i=0;i<VISIBLE_BLOCKS-1;++i){
			nextBlock[i] = nextBlock[i+1];
		}
		nextBlock[VISIBLE_BLOCKS-1] = rand()%7;
		recommend();
		DrawNextBlock(nextBlock);
		PrintScore(score);
		DrawField();
		DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
		timed_out=0;
	}
}

void recommendedPlay(){
	int command;
	clear();
	act.sa_handler = updateField;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(command=='q' || command=='Q'){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
}
