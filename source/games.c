#include <stdlib.h>
#include <ctype.h>
#define _GAMES_SOURCE
  #include "protos.h"
#undef _GAMES_SOURCE

#define GAMELIBID "Moesoft Game Library v1.22 (C)1997-1999 RaMSoft"

/*************** Poker, derived from Robb Thomas' version********************/
/* Get po_game struct pointer from name                                     */
/****************************************************************************/

struct po_game *get_po_game(char *name)
{
struct po_game *game;
name[0]=toupper(name[0]);
/* Search for exact name */
for(game=po_game_first;game!=NULL;game=game->next) {
	if (!strcmp(game->name,name)) return game;
	}
/* Search for close match name */
for(game=po_game_first;game!=NULL;game=game->next) {
	if (strstr(game->name,name)) return game;
	}
return NULL;
}

/*** Create a poker game ***/
struct po_game *create_po_game(void)
{
struct po_game *game;

if ((game=(struct po_game *)malloc(sizeof(struct po_game)))==NULL) {
        write_syslog("ERROR: Memory allocation failure in create_po_game().\n",0);
	return NULL;
	}

/* Append object into linked list. */
if (po_game_first==NULL) {  
        po_game_first=game;  game->prev=NULL;  
        }
   else {  
        po_game_last->next=game;  game->prev=po_game_last;  
	}
game->next=NULL;
po_game_last=game;

/* initialise the game */
game->name[0]='\0';
game->room=NULL;
game->players=NULL;
game->dealer=NULL;
game->newdealer=0;
game->num_players=0;
game->num_raises=0;
game->top_card=0;
game->bet=0;
game->pot=0;
game->state=0;
game->curr_player=NULL;
game->first_player=NULL;
game->last_player=NULL;

return game;
}

/*** Destruct a poker game. ***/
void destruct_po_game(struct po_game *game)
{
/* Remove from linked list */
if (game==po_game_first) {
        po_game_first=game->next;
        if (game==po_game_last) po_game_last=NULL;
        else po_game_first->prev=NULL;
	}
   else {
	game->prev->next=game->next;
        if (game==po_game_last) { 
                po_game_last=game->prev;  po_game_last->next=NULL; 
		}
                else game->next->prev=game->prev;
                }
sprintf(text, "~OL~FW-> ~FTGame ~FM%s~FT has ended.\n", game->name);
write_room(game->room, text);
free(game);
}

/*** Create a poker player ***/
struct po_player *create_po_player(struct po_game *game)
{
struct po_player *player;

if ((player=(struct po_player *)malloc(sizeof(struct po_player)))==NULL) {
        write_syslog("ERROR: Memory allocation failure in create_po_player().\n",0);
	return NULL;
	}

/* Append object into linked list. */
if (game->first_player==NULL) {
	game->first_player=player;  player->prev=NULL;  
	}
   else {  
	game->last_player->next=player;  player->prev=game->last_player;  
	}
player->next=NULL;
game->last_player=player;

/* Keep track of num players */
game->num_players++;

/* initialise the player */
player->hand[0] = -1;
player->touched = 0;
player->putin = 0;
player->rank = 0;
player->user = NULL;
player->game = game;
return player;
}

/*** Destruct a poker player. ***/
void destruct_po_player(struct po_player *player)
{
  struct po_game *game = player->game;
   /* if there are other players   */
   /* pass the turn before leaving */

  /* Keep track of num players */
  game->num_players--;

/*  if (game->num_players<=0){destruct_po_game(game);} */

  sprintf(text, "~OL~FW-> ~FTYou leave the game~FG %s~FT.\n", game->name);
  write_user(player->user, text);
  sprintf(text, "~OL~FW-> ~FTPlayer~FG %s~FT leaves the game~FG %s~FT.\n", player->user->name,game->name);
  write_room_except(player->user->room, text, player->user);

if (game->num_players > 1) {
	if (player->hand[0] != -1) {
        word_count = 1;
        fold_poker(player->user,NULL);
	}
	
	/* Pass the turn */
	if (game->curr_player == player)
        next_po_player(game);   
	
	/* Pass the honor of dealing */
	if (game->dealer == player)
        pass_the_deal(game);
}

/* Remove from linked list */
  if (player==game->first_player) {
	game->first_player=player->next;
	if (player==game->last_player) game->last_player=NULL;
	else game->first_player->prev=NULL;
        }
   else {
	player->prev->next=player->next;
	if (player==game->last_player) { 
        game->last_player=player->prev; game->last_player->next=NULL; 
	}
	else player->next->prev=player->prev;
  }
record(game->room, text);

if (game->state == 0) {
	game->curr_player = game->dealer;
        sprintf(text, "~OL~FW-> ~FGIt's your turn to deal.\n");
	write_user(game->dealer->user, text);
        sprintf(text, "~OL~FW-> ~FTIt's~FG %s's~FT turn to deal.\n",game->dealer->user->name);
	write_room_except(game->room, text, game->dealer->user);
	record(game->room, text);
}
free(player);
/* If the last player left, destruct the game */
if (game->first_player==NULL)
     destruct_po_game(game);
     }

/*** Shuffle cards ***/
void shuffle_cards(int deck[])
{
int i, j, k, tmp;
  
/* init the deck */
  for (i = 0; i < 52; i++) 
	deck[i] = i;

/* do this 7 times */
  for (k = 0; k < 7; k++) {
	/* Swap a random card below */
	/* the ith card with the ith card */
	for (i = 0; i < 52; i++) {
        j = myRand(52-i);
        tmp = deck[j];   
        deck[j] = deck[i];
        deck[i] = tmp;
	}
   }
}

/*** look at hand ***/
void hand_poker(UR_OBJECT user)
{
if (user->pop == NULL) {
        write_user(user, "~OL~FRYou must be in a poker game first!\n");
	return;
        }
if (user->pop->hand[0] == -1) {
        write_user(user, "~OL~FW-> ~FRYou need to have some cards first.\n");
	return;
        }
print_hand(user, user->pop->hand);
}

/*** Print hand ***/
void print_hand(UR_OBJECT user,int hand[])
{
int i, j;
sprintf(text, " ");
        for (i = 0; i < CARD_LENGTH; i++) {
	for (j = 0; j < 5; j++) {
		if (user->high_ascii) strcat(text, cards_ansi[hand[j]][i]);
		else strcat(text, cards[hand[j]][i]);
                strcat(text, "   ");
                }
                strcat(text, "\n");
                write_user(user,text);
                sprintf(text, " ");
                }
/** Print the labels */
write_user(user, "~OL~FW<-- ~FT1~FW --> <-- ~FT2~FW --> <-- ~FT3~FW --> <-- ~FT4~FW --> <-- ~FT5~FW -->\n");
}

/*** Print hand for the room ***/
void room_print_hand(UR_OBJECT user,int hand[])
{
int i, j;
sprintf(text, " ");
        for (i = 0; i < CARD_LENGTH; i++) {
	for (j = 0; j < 5; j++) {
        strcat(text, cards[hand[j]][i]);
        strcat(text, "   ");
	}
	strcat(text, "\n");
	write_room(user->room,text);
	record(user->room, text); 
	sprintf(text, " ");
        }
}

/*** Start a poker Game ***/
void start_poker(UR_OBJECT user)
{
struct po_game *game;
int hist_index;

if ((hist_index = get_player_hist_index(user)) == -1) {
        write_user(user,"~OL~FRYou need some money first!\n");
	return;
        }
if (word_count < 2) {
        write_user(user,"~OL~FRYou need to name the game.\n");
	return;
        }
if (strlen(word[1])>=15) {
        write_user(user,"~OL~FRYou'll need to pick a shorter game name than that!\n");
        return;
        }
if ((game=get_po_game(word[1]))!=NULL) {
        write_user(user,"~OL~FRThere is already a game with that name.\n");
	return;
        }
if ((game=get_po_game_here(user->room))!=NULL) {
        write_user(user,"~OL~FTThere is already a game in this room.\n");
	return;
        }
/*if ((cwgame=get_cw_game(word[1]))!=NULL) {
	write_user(user, "~OL~FRThere is already a game with that name.\n");
	return;
        }
if ((cwgame=get_cw_game_here(user->room))!=NULL) {
	write_user(user, "~OL~FRThere is already a game in this room.\n");
	return;
        } */
if (user->pop == NULL) { 
        if ((game=create_po_game())==NULL) {
        write_syslog("ERROR: Memory allocation failure in start_poker().\n",0);
        write_user(user, "~FRSorry, can't start a game, No Resources Left!\n");
        return;
	}
   else {
        if ((user->pop=create_po_player(game))==NULL) {
                write_syslog("ERROR: Memory allocation failure in start_poker().\n",0);
                write_user(user, "~OL~FRSorry, can't start a game, No resources left!\n");
		return;
                }
        user->pop->user = user;
        user->pop->hist = po_hist[hist_index];
        game->players = user->pop;
        game->curr_player = user->pop;
        game->dealer = user->pop;
        strcpy(game->name, word[1]);
        game->room = user->room;
        sprintf(text, "~OL~FW-> ~FGYou start a game of Poker called~FM %s.\n", game->name);
        write_user(user,text);
        sprintf(text, "~OL~FW<~FG %s~FW started a game of Poker called~FG %s~FW >\n",user->name, game->name);
        write_room_except(user->room,text, user);
        write_user(user, "~OL~FW-> ~FGIt's your turn to deal.\n");
        }
}
   else {
        write_user(user, "~OL~FRYou are already in a game.\n");
        }
}

/*** Join a poker game ***/
void join_poker(UR_OBJECT user)
{
struct po_game *game;
int hist_index;
if ((hist_index = get_player_hist_index(user)) == -1) {
        write_user(user, "~OL~FTYou need money!  ~FMAsk a wiz!\n");
	return;
        }
/*if ((user->pop != NULL)&&(user->cwp != NULL)) {
	write_user(user, "~OL~FRYou are already in a game.\n");
	return;
        }*/
if ((user->pop != NULL)&&(user->pop != NULL)) {
	write_user(user, "~OL~FRYou are already in a game.\n");
	return;
        }
if ((game=get_po_game_here(user->room))==NULL) {
        write_user(user, "~OL~FTYou gotta be in the same room as the game.\n");
	return;
        }
if (game->num_players == 6) {
        write_user(user, "~OL~FTThis Poker game is full.\n");
	return;
        }
if ((user->pop=create_po_player(game))==NULL) {
	write_syslog("ERROR: Memory allocation failure in join_po().\n",0);
        write_user(user, "~OL~FRSorry, can't join the game!\n");
	return;
        }
   else {
	user->pop->user = user;
        user->pop->hist = po_hist[hist_index];
        sprintf(text, "~OL~FW-> ~FGYou join the game~FG %s.\n", game->name);
	write_user(user, text);
        sprintf(text, "~OL<~FG %s~FW joins the game~FG %s~FW >\n", user->name, game->name);
	write_room_except(user->room, text, user);
	record(user->room, text); 
        }
}

/*** Leave a PO Game ***/
void leave_poker(UR_OBJECT user)
{
if (user->pop != NULL) {
        destruct_po_player(user->pop);
	user->pop = NULL;
        }
   else {
        write_user(user, "~OL~FRYou aren't in a Poker game!\n");
        }
}

/*** List PO Games ***/
void list_po_games(UR_OBJECT user)
{
struct po_game *game;
int count = 0;
        write_user(user,"\n~OL~FM-~FR=~FM- ~FGCurrent Poker games being played ~FM-~FR=~FM-\n\n");
        write_user(user,"~FY~OLName            : Room                 : # of Players\n");
        for(game=po_game_first;game!=NULL;game=game->next) {
        sprintf(text,"~OL%-15s : %-20s : %d players\n",game->name,game->room->name,game->num_players);
	write_user(user, text);
	count++;
        }
if (!count) sprintf(text,"\n~OL~FRThere are no games currently being played!\n\n");
else if (count==1) sprintf(text,"\n~OLTotal of~FM %d~FW game being played.\n\n", count);
else sprintf(text,"\n~OLTotal of~FM %d~FW games being played.\n\n", count);
write_user(user, text);
}

/*** add_player_hist ***/
int add_player_hist(UR_OBJECT u,int total,int given)
{
struct po_player_hist *player_hist;

if ((player_hist=(struct po_player_hist *)malloc(sizeof(struct po_player_hist)))==NULL) {
        write_syslog("ERROR: Memory allocation failure in add_player_hist().\n",0);
	return 0;
        }

/* create a new po player _history */
strcpy(player_hist->name, u->name);
player_hist->total = total;
player_hist->given = given;

/* add it to the list */
po_hist[max_hist] = player_hist;
max_hist++;
sort_hist();
return 1;
}

/*** get index for a player _hist */
int get_player_hist_index(UR_OBJECT u)
{
int i;
for (i = 0; i < max_hist; i++) {
        if (strcmp(po_hist[i]->name, u->name) == 0) {
        return i;
	}
}
return -1;
}

/*** po player history compare ***/
int player_hist_cmp(struct po_player_hist **a,struct po_player_hist **b)
{
int a_winnings;
int b_winnings;

a_winnings = (*a)->total;
b_winnings = (*b)->total;
if (a_winnings < b_winnings) { return 1; }
else if (a_winnings > b_winnings) { return -1; }
return 0;
} 									 

/*** sort the po _history ***/
void sort_hist(void)
{
if (max_hist > 0) {
        qsort(po_hist, max_hist, sizeof(po_hist[0]),
                  (int (*) (const void *, const void *))player_hist_cmp);
                  }
}

/*** show po player history ***/
void rank_po(UR_OBJECT user)
{
int i;
int many = 0;

if (word_count == 2) {
	many = atoi(word[1]);
        }
        if (many < 5) many = 5;
        if (many > max_hist) many = max_hist;
	write_user(user,"~FG=-=-=-~OL~FB Island Casino ~FGPoker Ranking!~RS~FG -=-=-=\n");
	write_user(user,"~FM~OLRank: Name            : Total             \n");
	write_user(user,"~FG-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
        for (i = 0; i < many; i++) {
        sprintf(text,"  %-2d: %-15.15s : $%-17d\n",i+1,po_hist[i]->name,po_hist[i]->total);
	write_user(user, text);
        }
for (i = 0; i < max_hist; i++) {
        if (!strcmp(user->name, po_hist[i]->name)) {
        if (i == many) {
        sprintf(text,"  %-2d: %-15.15s : $%-17d\n",i+1,po_hist[i]->name,po_hist[i]->total);
		write_user(user, text);
		break;
                }
        else if (i > many) {
                sprintf(text, "           :\n  %-2d: %-15.15s : $%-17d\n",i+1,po_hist[i]->name,po_hist[i]->total);
		write_user(user, text);
		break;
                }
           else {
		break;
                }
	}
  }	
write_user(user,"~FG=-=-=-~OL~FB Island Casino ~FGPoker Ranking!~RS~FG -=-=-=\n");
}

/*** Give chips to all the little people ***/
void chips_po(UR_OBJECT user)
{
int amount=0;
int hist_index=0;
int type=0;
int balance=0;
  
if (word_count<3) {
        write_user(user,"Usage: chips [buy]/[sell] <amount>\n");
	return;
        }
balance=user->bank_balance;
amount = atoi(word[2]);
if (amount%5>0) { 
	write_user(user,"~OL~FRWe only cary $5 and $10 chips here!\n");
	return;
	}
if (amount<10) {
	write_user(user,"~OL~FRThe ammount must be more than ~FT$10.00~FR!\n");
	return;
	}
if (!strcasecmp(word[1],"buy")) type=1;
if (!strcasecmp(word[1],"sell")) type=2;
if (!type) {
	write_user(user,"~OL~FRYou must ~FB'~FMbuy~FB' ~FRor ~FB'~FMsell~FB' ~FRchips!\n");
	return;
	}
if (!balance && type==1) {
	write_user(user,"~OL~FRYou do not have any money in the bank to buy chips!\n");
	return;
	}
if (type==1 && (balance<amount)) {
	write_user(user,"~OL~FRYou don't have that kind of money in the bank!\n");
	return;
	}
if (balance<30 && type==1) {
	sprintf(text,"~OL~FGYour ~FY%s~FG Bank Balance = ~FT$%d.00\n",TALKERNAME,balance);
	write_user(user,text);
	write_user(user,"~OL~FRYou *must* have atleast $30 to buy chips.\n");
	write_user(user,"~OL~FMYou can either bug a wiz, ask a user for a loan, or try your luck\n~OL~FMAt the craps table!\n");
	return;
	}
if (amount<30 && type==1) {
	write_user(user,"~OL~FRYou cannot buy any less than $30 in chips!\n");
	return;
	}
/* add to player history */
if ((hist_index=get_player_hist_index(user))==-1) {
	/* not in _hist */
	if (type==2) {
		amount=0;
		write_user(user,"~OL~FRYou have no poker chips to sell!\n");
		return;
		}
	if (add_player_hist(user,amount,amount)) hist_index=get_player_hist_index(user);
	else return;
	}
else {
	/* already in _hist */
	if (type==2) {
		if (amount>po_hist[hist_index]->total) {
			write_user(user,"~OL~FRYou cannot sell more chips than you have! ~FM*Nice try* ;)\n");
			return;
			}
		else {
	        	po_hist[hist_index]->total -= amount;
        		po_hist[hist_index]->given -= amount;
			}	
		}
	else {
        	po_hist[hist_index]->total += amount;
        	po_hist[hist_index]->given += amount;
		}
        sort_hist();
        }
if (type==1) withdrawlbank(user,amount,0);
else depositbank(user,amount,0);
balance=user->bank_balance;
sprintf(text,"~OL~FGYou now have ~FT$%d.00 ~FGin poker chips and ~FT$%d.00 ~FGin the bank.\n",po_hist[hist_index]->total,balance);
write_user(user,text);
}

/*** Give chips to all the little people ***/
void give_chips_po(UR_OBJECT user)
{
UR_OBJECT u;
char text2[80];
int amount;
int hist_index;
  
if (word_count<2) {
        write_user(user,"Usage: chips <user> <amount>\n");
	return;
        }
amount = atoi(word[2]);
if (amount > 500) {
        write_user(user,"An amount lower then 500 please!\n");
        return;
        }
if (amount < 50) {
	write_user(user,"An amount more than 50 please!\n");
	return;
	}
/* User currently logged in */
if ((u=get_user(word[1]))) {
	/* add to player history */
        if ((hist_index=get_player_hist_index(u))==-1) {
        /* not in _hist */
        if (add_player_hist(u,amount,amount))
                hist_index=get_player_hist_index(u);
            else
		return;
                }
           else {
	/*sprintf(text, "hist_index = %d\n", hist_index);
  	write_user(user, text);*/
	/* already in _hist */
        po_hist[hist_index]->total += amount;
        po_hist[hist_index]->given += amount;
        sort_hist();
        }
	sprintf(text,"~FG~OL%s~FW has~FG $%d~FW poker chips.\n",u->name,po_hist[hist_index]->total);
	write_user(user,text);
        sprintf(text,"~FG~OL%s~FW has given you~FG $%s~FW worth of poker chips.\n",user->name,word[2]);
	write_user(u,text);
	sprintf(text,"%s gave %s $%s worth of poker chips.\n",user->name,u->name,word[2]);
	write_syslog(text,1);
	return;
        }
	/* User not logged in */
if ((u=create_user())==NULL) {
	sprintf(text,"~FR%s: unable to create temporary user object.\n",syserror);
	write_user(user,text);
        write_syslog("ERROR: Unable to create temporary user object in chips_po().\n",0);
	return;
        }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
	write_user(user,nosuchuser);
	destruct_user(u);
	destructed=0;
	return;
        }
	/* add to player history */
        if ((hist_index = get_player_hist_index(u)) == -1) {
	/* not in _hist */
        if (add_player_hist(u,amount,amount))
        hist_index = get_player_hist_index(u);
   else {
        destruct_user(u);
        destructed=0;
        return;
        }
        } else {
	/* already in _hist */
        po_hist[hist_index]->total += amount;
        po_hist[hist_index]->given += amount;
        sort_hist();
        }
sprintf(text,"~FG~OL%s~FW has~FG $%d~FW poker chips.\n",word[1],po_hist[hist_index]->total);
write_user(user,text);
u->socket=-2;
strcpy(u->site,u->last_site);
save_user_details(u,0);
sprintf(text2,"~OLYou have been given~FG $%s~FW worth of poker chips.~RS\n",word[2]);
send_mail(user,word[1],text2);
sprintf(text,"%s gave %s $%s poker chips.\n",user->name,u->name,word[2]);
write_syslog(text,1);
destruct_user(u);
destructed=0;
}

/*** save po history to disk ***/
int save_hist(void)
{
FILE *fp;
char filename[80];
int i;
sprintf(filename,"%s/%s",MISCFILES,PO_HIST_FILE);
if (!(fp=fopen(filename,"w"))) {
        sprintf(text,"Save_hist(): Failed to save poker history.\n");
	write_syslog(text,1);
	return 0;
        }
fprintf(fp,"%d\n",max_hist);
for (i = 0; i < max_hist; i++) {
        fprintf(fp,"%s\n",po_hist[i]->name);
        fprintf(fp,"%d %d\n",po_hist[i]->total,po_hist[i]->given);
        }
fclose(fp);
return 1;
}

/*** load poker history from disk ***/
int load_hist(void)
{
FILE *fp;
char filename[80];
int i;
struct po_player_hist *player_hist;
  
sprintf(filename,"%s/%s",MISCFILES,PO_HIST_FILE);
if (!(fp=fopen(filename,"r"))) {
        printf(">> Load_hist: Failed to load poker history.\n");
	return 0;
        }
fscanf(fp,"%d",&max_hist); 
printf(">> max_hist = %d\n", max_hist);
for (i = 0; i < max_hist; i++) {
        if ((player_hist=(struct po_player_hist *)malloc(sizeof(struct po_player_hist)))==NULL) {
        	printf(">> ERROR: Memory allocation failure in load_hist().\n");
        	return 0;
		}
	fscanf(fp,"%s",player_hist->name);
	fscanf(fp,"%d %d",&(player_hist->total),&(player_hist->given));
	printf(">> User Name: %-20s  Total: $%d\n",player_hist->name,player_hist->total);
	/* add it to the list */
	po_hist[i]=player_hist;
	}
fclose(fp);
sort_hist();
printf(">> Loaded poker history.\n");
return 1;
}

/*** Deal cards to all players ***/
void deal_poker(UR_OBJECT user)
{
int i;
struct po_game *game;
struct po_player *tmp_player;

if (user->pop == NULL) {
        write_user(user,"~OL~FRYou have to be in a Poker game to use this command.\n");
	return;
        }
game = user->pop->game;
if (user->pop != game->dealer) {
        sprintf(text,"~OL~FW-> ~FG%s~FW is the dealer.\n",game->dealer->user->name);
	write_user(user, text);
	return;
        }
if (game->state != 0) {
        write_user(user,"~OL~FW-> ~FRYou can't deal now.\n");
	return;
        } 
if (game->num_players < 2) {
        write_user(user,"~OL~FW-> ~FRYou need two people in to deal poker.\n");
	return;
        } 
/* Reset game state */
        game->top_card = 0;
        game->bet = 0;
        game->pot = 0;
        game->num_raises = 0;
        game->in_players = 0;
        game->opened = NULL;
        game->newdealer = 0;
  
shuffle_cards(game->deck);

/* Reset players */
for (tmp_player = game->first_player; tmp_player != NULL;
        tmp_player = tmp_player->next) {
        if (tmp_player->hist->total >= 5) {
        	tmp_player->hand[0] = -1;
        	tmp_player->touched = 0;
        	tmp_player->putin = 5;
        	tmp_player->rank = 0;
        	game->in_players++;
		write_user(tmp_player->user,"~OL~FW-> ~FGYou ante $5.\n");
        	}
	else {
		write_user(tmp_player->user,"~OL~FW-> ~FRYou need more poker chips!\n");
		}
	}
sort_hist();
if (game->in_players<2) {
        write_room(user->room,"~OL~FTNot enough people have chips to ante.  You need at least two.\n");
	record(user->room,"~FRNot enough people have chips to ante.  You need at least two.\n"); 
	return;
        }
write_room(user->room,"~OL~FW-> ~FTEveryone has anted $5.\n");
record(user->room, "~OL~FW-> ~FTEveryone has anted $5.\n"); 
write_user(user, "~OL~FW-> ~FGYou shuffle and deal the cards.\n");
sprintf(text, "~OL~FW-> ~FG%s~FW shuffles and deals the cards.\n", user->name);
write_room_except(user->room,text,user);
record(user->room,text); 
/* Start with the player to the left of the dealer */
game->curr_player = game->dealer;
next_po_player(game);
tmp_player = game->curr_player;

/* deal five to each player */
for (i = 0; i < 5; i++) {
	do {
		if (game->curr_player->putin == 5) {
	  	game->curr_player->hand[i] = game->deck[game->top_card];
	  	game->top_card++;
	if (i == 4) {
     		game->curr_player->hist->total -= 5;
		game->pot += 5;
		game->curr_player->putin = 0;	
		print_hand(game->curr_player->user,game->curr_player->hand);
		}
	}
/* Next Player */
next_po_player(game);
	}
	while (game->curr_player != tmp_player);
	}
/* Make this guy the default opened guy */
game->opened = game->curr_player;
write_user(game->curr_player->user,"~OL~FW-> ~FTThe first round of betting starts with you.\n");
sprintf(text,"~OL~FW-> ~FTThe first round of betting starts with ~FG%s~FT.\n",game->curr_player->user->name);
write_room_except(user->room,text,game->curr_player->user);
record(user->room,text);
game->state = 1;
}

/*** Next poker player ***/
void next_po_player(struct po_game *game)
{
if (game->curr_player->next == NULL) {
	game->curr_player = game->first_player;
        }
   else {
	game->curr_player = game->curr_player->next;
        }  
}

/*** Next in player ***/
void next_in_player(struct po_game *game)
{
do {
	if (game->curr_player->next == NULL) {
        game->curr_player = game->first_player;
	}
   else {
        game->curr_player = game->curr_player->next;
	}  
    } while (game->curr_player->hand[0] == -1);
}

/*** Bet in poker ***/
void bet_poker(UR_OBJECT user)
{
struct po_game *game;
int player_bet;
if (user->pop == NULL) {
        write_user(user,"~OL~FRYou have to be in a Poker game to use this command.\n");
	return;
        }
game = user->pop->game;

/* Check if it's possible to bet */
if ((game->state != 1)&&(game->state != 3)) {
        write_user(user,"~OL~FW-> ~FRYou can't bet now.\n");
	return;
        } 
if (game->curr_player != user->pop) {
        write_user(user,"~OL~FW-> ~FRYou can't bet unless it's your turn.\n");
	return;
        }
if (word_count < 2) {
        write_user(user,"~OL~FW-> ~FGHow much do you want to bet?\n");
	return;
        }
player_bet = atoi(word[1]);
if ((player_bet == 0) && (word[1][0] != '0')) {
        write_user(user,"~OL~FW-> ~FRPlease use numbers.\n");
	return;
        }
if (player_bet < 0) {
        write_user(user,"~OL~FW-> ~FRYou must bet atleast $5!\n");
	return;
        }
if (player_bet%5 != 0) {
        write_user(user,"~OL~FW-> ~FYMake your bet a multiple of $5 please.\n");
	return;
        }
if (player_bet == 0) {
        check_poker(user);
	return;
        }
if (player_bet < game->bet) {
        sprintf(text,"~OL~FW-> ~FYYou must bet at least~FG $%d~FW or fold.\n",game->bet);
	write_user(user, text);
	return;
        }
if (player_bet - game->bet > 100) {
	if (game->bet == 0) {
        write_user(user,"~OL~FW-> ~FYThe largest opening bet is $100.\n");
        return;
	}
   else {
        write_user(user,"~OL~FW-> ~FYThe largest raise is $100.\n");
        return;
	}
}
if ((player_bet > game->bet) && (game->bet != 0)) {
        if (game->num_raises > 5) {
        write_user(user,"~OL~FW-> ~FRThere is a limit of five raises.  Please see the bet, or fold.\n");
        return;
	}
	game->num_raises++;
        }
if (user->pop->hist->total < player_bet) {
        sprintf(text,"~OL~FW-> ~FRYou don't have enough chips to make that bet.\n");
	write_user(user, text);
	return;
        } 
bet_po_aux(user,player_bet);
}

/*** Aux bet function ***/
void bet_po_aux(UR_OBJECT user,int player_bet)
{
struct po_game *game;
game = user->pop->game;
user->pop->touched = 1;
if (player_bet == game->bet) {
        sprintf(text, "~OL~FW-> ~FTYou see the bet of~FG $%d\n", game->bet);
	write_user(user, text);
        sprintf(text, "~OL~FW->~FG %s~FT sees the bet of~FG $%d\n",user->name,game->bet);
	write_room_except(user->room, text, user);
	record(game->room,text); 
        }
else if (game->bet == 0) {
	game->opened = user->pop;
        sprintf(text,"~OL~FW-> ~FMYou open the betting with~FG $%d\n",player_bet);
	write_user(user, text);
        sprintf(text,"~OL~FW->~FG %s~FT opens the betting with~FG $%d\n",user->name,player_bet);
        write_room_except(user->room,text,user);
	record(game->room,text); 
        }
   else {
        sprintf(text, "~OL~FW-> ~FMYou raise the bet to~FG $%d~FM.\n",player_bet);
	write_user(user, text);
        sprintf(text, "~OL~FW-> ~FG%s~FW raises the bet to~FG $%d\n",user->name,player_bet);
	write_room_except(user->room, text, user);
	record(game->room,text); 
        }
game->bet = player_bet;
game->pot += (player_bet - user->pop->putin);
user->pop->hist->total -= (player_bet - user->pop->putin);
user->pop->putin = player_bet;
sort_hist();

sprintf(text, "~OL~FW-> ~FTThe pot is now~FG $%d.\n",game->pot);
write_user(user, text);

/* Go to next elegible player */
next_in_player(game);

/* Check if all players have called */
all_called_check(game);
bet_message(game);
}

/*** Raise the bet in poker ***/
void raise_poker(UR_OBJECT user,char *inpstr)
{
struct po_game *game;
int player_raise;
if (user->pop == NULL) {
        write_user(user,"~OL~FRYou have to be in a Poker game to use this command.\n");
	return;
        }
game = user->pop->game;

/* Check if it's possible to raise */
if ((game->state != 1)&&(game->state != 3)) {
        write_user(user,"~OL~FW-> ~FRYou can't raise now.\n");
	return;
        } 
if (game->curr_player != user->pop) {
        write_user(user,"~OL~FW-> ~FRYou can't raise unless it's your turn.\n");
	return;
        }
if (word_count < 2) {
        write_user(user,"~OL-> ~FGBy how much do you want to raise?\n");
	return;
        }
player_raise = atoi(word[1]);
if ((player_raise == 0) && (word[1][0] != '0')) {
        write_user(user,"~OL~FRYou gotta use numbers!\n");
	return;
        }
if (player_raise < 0) {
        write_user(user,"~OL~FRUh?  Your raise must be atleast $10!\n");
	return;
        }
if (player_raise%5 != 0) {
        write_user(user,"~OL~FRMake your raise a multiple of $5 please.\n");
	return;
        }
if (player_raise < 10) {
        write_user(user,"~OL~FRThe smallest raise is $10.\n");
	return;
        }
if (player_raise > 100) {
        write_user(user,"~OL~FRThe largest raise is $100.\n");
	return;
        }
if (player_raise == 0) {
        write_user(user,"~OL~FRThe smallest raise is $10.\n");
	return;
        }
if (game->num_raises > 5) {
        write_user(user,"~OL~FRThere is a limit of five raises.  Please see the bet or fold.\n");
	return;
        }
player_raise += game->bet;
game->num_raises++;
if (user->pop->hist->total < player_raise) {
        write_user(user,"~OL~FRYou don't have enough chips to make that raise.\n");
	return;
        }
bet_po_aux(user, player_raise);
}

/*** see a bet ***/
void see_poker(UR_OBJECT user)
{
struct po_game *game;
int player_bet;
if (user->pop == NULL) {
        write_user(user,"~OL~FRYou have to be in a Poker game to use this command.\n");
	return;
        }
game = user->pop->game;

/* Check if it's possible to see a bet */
if ((game->state != 1)&&(game->state != 3)) {
        write_user(user,"~OL~FW-> ~FRYou can't see a bet now.\n");
	return;
        } 
if (game->curr_player != user->pop) {
        write_user(user,"~OL~FW-> ~FRYou can't see a bet unless it's your turn.\n");
	return;
        }
player_bet = game->bet;
if (user->pop->hist->total < player_bet) {
        write_user(user,"~OL~FW-> ~FRYou don't have enough chips to see that bet.\n");
	return;
        }
if (player_bet == 0)
        check_poker(user);
else
        bet_po_aux(user, player_bet);
        }

/*** All called check ***/
void all_called_check(struct po_game *game)
{
struct po_player *tmp_player;
if (game->curr_player->touched && game->curr_player->putin == game->bet) {
	/* Everyone has called */
	/* reset the touched flags and putin ammts */
for (tmp_player = game->first_player; tmp_player != NULL;
          tmp_player = tmp_player->next) {
	  tmp_player->touched = 0;
	  tmp_player->putin = 0;
          }
game->state++; /* next state */
switch (game->state) {
    case 2:
	  /* Start with the player to the left of the dealer */
	  game->curr_player = game->dealer;
	  next_in_player(game);
          hand_poker(game->curr_player->user);
          sprintf(text,"~OL~FW-> ~FGIt's your turn to discard.\n~OL~FW-> ~FMUse: .discpo [# # ...]\n");
	  write_user(game->curr_player->user, text);
          sprintf(text,"~OL~FW-> ~FTIt's~FG %s's~FT turn to discard.\n",game->curr_player->user->name);
          write_room_except(game->room,text,game->curr_player->user);
	  record(game->room, text); 
	  break;
    case 4:
          write_room(game->room,"~OL~BM~FY!!! SHOWDOWN !!!~RS\n");
          showdown_poker(game);
	  break;
          }
     }
}

/*** pass the deal to the next player ***/
void pass_the_deal(struct po_game *game)
{
if (game->dealer->next == NULL) {
	game->dealer = game->first_player;
        }
   else {
	game->dealer = game->dealer->next;
        }
if (game->state != 0) {
	game->newdealer = 1;  /* mark that we have a new dealer */
        }
}

/*** Fold and pass turn to next player ***/
void fold_poker(UR_OBJECT user,char *inpstr)
{
struct po_game *game;
struct po_player *tmp_player;

if (user->pop == NULL) {
        write_user(user,"~OL~FRYou have to be in a Poker game to use this command.\n");
	return;
        }
if (user->pop->hand[0] == -1) {
        write_user(user,"~OL~FW-> ~FRYou don't have any cards.\n");
	return;
        }
game = user->pop->game;

/* Mark folding by making the first card in hand -1 */
user->pop->hand[0] = -1;
game->in_players--;
if (word_count < 2) {
        sprintf(text,"~OL~FW-> ~FGYou fold.\n");
	write_user(user, text);
        sprintf(text,"~OL~FW->~FG %s~FT folds.\n", user->name);
        }
   else {
        sprintf(text,"~OL~FW-> ~FTYou say, ~FB\"~FM%s~FB\" ~FTand fold.\n", inpstr);
	write_user(user, text);
        sprintf(text,"~OL~FW-> ~FG%s~FT says, ~FB\"~FM%s~FB\" ~FTand folds.\n",user->name,inpstr);
        }
write_room_except(user->room, text, user);
record(user->room,text); 
/* Check if there is 1 player in */
if (game->in_players == 1) {
	/* The in person wins */
	next_in_player(game);
	/* add to players total_bux */
     game->curr_player->hist->total += game->pot;
        sort_hist();
        sprintf(text,"~OL~FW-> ~FGYou win~FG $%d~FW!!!\n", game->pot);
	write_user(game->curr_player->user, text);
        sprintf(text,"~OL~FW-> ~FG%s~FT wins~FG $%d~FT!!!\n", 
                game->curr_player->user->name, game->pot);
                write_room_except(game->room,text,game->curr_player->user);
                record(game->room, text);
	game->pot = 0;
	game->bet = 0;
	game->state = 0;  /* reset and deal cards */
	for (tmp_player = game->first_player; tmp_player != NULL;
          tmp_player = tmp_player->next) {
	  /* clear players' hands */
	  tmp_player->hand[0] = -1;
          } 
/* pass the deal if it hasn't already */
if (!game->newdealer)
        pass_the_deal(game);
        game->curr_player = game->dealer;
        sprintf(text,"~OL-> It's your turn to deal.\n");
	write_user(game->dealer->user, text);
        sprintf(text,"~OL~FT It's~FG %s's~FT turn to deal.\n",game->dealer->user->name);
                        write_room_except(game->room,text,game->dealer->user);
                        record(game->room, text);
                        }
                   else {
			/* check if it's my turn */
                        if (game->curr_player == user->pop) {
			/* Go to next elegible player */
                        next_in_player(game);
			/* Check what state we're in */
                        all_called_check(game);
                        bet_message(game);
                        }
           }
}

/*** Check poker (kinda lame command, but I decided to keep it in ***/
void check_poker(UR_OBJECT user)
{
struct po_game *game;
if (user->pop == NULL) {
        write_user(user,"~OL~FRYou have to be in a Poker game to use this command.\n");
	return;
        }
game = user->pop->game;

/* Check if it's possible to check */
if ((game->state != 1)&&(game->state != 3)) {
        write_user(user,"~OL~FW-> ~FRYou can't check now.\n");
	return;
        } 
if (game->curr_player != user->pop) {
        write_user(user,"~OL~FW-> ~FRYou can't check unless it's your turn.\n");
	return;
        }
if (game->bet > 0) {
        sprintf(text,"~OL~FW-> ~FRYou must bet at least~FG $~FT%d~FR or fold.\n",game->bet);
	write_user(user, text);
	return;
        }
/* We've checked! */
user->pop->touched = 1;
sprintf(text,"~OL-> You check.\n");
        write_user(user, text);
        sprintf(text,"~OL~FW-> ~FG%s~FT checks.\n",user->name);
        write_room_except(user->room,text, user);
        record(game->room,text);

/* Go to next elegible player */
next_in_player(game);

/* Check what state we're in */
        all_called_check(game);
        bet_message(game);
        }

/*** Bet message ***/
void bet_message(struct po_game *game)
{
if (game->state == 1 || game->state == 3) {
        sprintf(text,"~OL~FW-> ~FTYou've put in~FG $%d~FT this round. The bet is~FG $%d ~FTto you.\n",game->curr_player->putin,game->bet);
	write_user(game->curr_player->user, text);
        sprintf(text,"~OL~FW-> ~FTIt's~FG %s's~FT turn to bet.\n",game->curr_player->user->name);
	write_room_except(game->room, text, game->curr_player->user);
	record(game->room,text);
        }
}

/*** Discard cards in your hand ***/
void disc_poker(UR_OBJECT user)
{
struct po_game *game;
struct po_player *tmp_player;
int i;
int have_ace;
int disc_these[5];
int choice;
if (user->pop == NULL) {
        write_user(user,"~OL~FRYou have to be in a Poker game to use this command.\n");
	return;
        }
game = user->pop->game;

/* Check if it's possible to discard */
if (game->state != 2) {
        write_user(user,"~OL-> ~FRYou can't discard now!\n");
	return;
        } 
if (game->curr_player != user->pop) {
        write_user(user,"~OL-> ~FRYou can't discard unless it's your turn!\n");
	return;
        }
if (word_count > 5) {
        write_user(user,"~OL-> ~FRYou can't discard five cards!\n");
	return;
        }
if (word_count > 4) {
	/* Look for an ace */
	have_ace = 0;
	i = 0;
     do {
        if (user->pop->hand[i]%13 == 12)
        have_ace = 1;
        i++;
	} while ((i < 5) && (!have_ace));
if (!have_ace) {
        write_user(user,"~OL~FW-> ~FRYou can't discard four cards unless you have an ace.\n");
        return;
        }
}
user->pop->touched = 1;
if ((word_count < 2) || (word[1][0] == '0')) {
	/* No discards */
        sprintf(text,"~OL~FW-> ~FGYou stand pat.\n");
	write_user(user, text);
        sprintf(text,"~OL~FW-> ~FG%s~FT stands pat.~RS\n",user->name);
	write_room_except(user->room, text, user);
	record(game->room,text); 
        }
   else {
	/* discards */
	/** Init the array **/
	for (i = 0; i < 5; i++)
        disc_these[i] = 0;
	/** Get which cards to discard **/
	for (i = 1; i < word_count; i++) {
        choice = atoi(word[i]);
if ((choice <= 0) || (choice > 5)) {
                sprintf(text,"~OL~FW!~FM> ~FRChoose a number ~FT1 ~FRthrough ~FT5 ~FRplease!\n");
		write_user(user, text);
		return;
                }
           else {
		disc_these[choice - 1] = 1; /* We're not keeping this one */
                }
	}
        /** draw cards **/
	for (i = 0; i < 5; i++) {
        if (disc_these[i]) {
		user->pop->hand[i] = game->deck[game->top_card];
		game->top_card++;
                }
	}
        sprintf(text,"~OL~FW-> ~FGYou have discarded~FG %d~FW card(s).\n",word_count-1);
	write_user(user, text);
        sprintf(text,"~OL~FW-> ~FG%s~FT has discarded~FG %d~FT card(s)~RS.\n",user->name,word_count-1);
	write_room_except(user->room, text, user);
	record(game->room,text);
        print_hand(user, user->pop->hand);
        }

/* Go to next elegible player */
next_in_player(game);
if (game->curr_player->touched) {
	/* We've dealt cards to everyone */
	/* reset the touched flags */
	for (tmp_player = game->first_player; tmp_player != NULL;
                tmp_player = tmp_player->next) {
                tmp_player->touched = 0;
                }
	/* reset the bet */
	game->bet = 0;
	game->num_raises = 0;
        /* Start with the player who opened if still in */
        game->curr_player = game->opened;
	if (game->curr_player->hand[0] == -1) {
        /* That person folded */
        next_in_player(game);
	}
        sprintf(text,"~OL~FW-> ~FTThe current pot is~FG $%d.\n",game->pot);
	write_room(user->room, text);
        sprintf(text,"~OL~FW-> ~FGThe second round of betting starts with you.\n");
	write_user(game->curr_player->user, text);
        sprintf(text,"~OL~FW-> ~FTThe second round of betting starts with~FG %s\n",game->curr_player->user->name);
	write_room_except(user->room, text, game->curr_player->user);
	record(game->room,text); 
        game->state = 3;
        }
   else {
        hand_poker(game->curr_player->user);
        sprintf(text,"~OL-> ~FGIt's your turn to discard.\n~OL~FW-> ~FMUse: ~FT.discpo ~FR# # ..., or just .discpo by itself to stand pat!\n");
	write_user(game->curr_player->user, text);
        sprintf(text,"~OL~FW-> ~FTIt's~FG %s's~FT turn to discard.\n",game->curr_player->user->name);
	write_room_except(user->room, text, game->curr_player->user);
	record(game->room,text);
        }
}

/*** Showdown ***/
void showdown_poker(struct po_game *game)
{
struct po_player *tmp_player;
struct po_player *winners[4];
int num_winners;
int i, j, temp;
int loot;
char rtext[20];
winners[0] = game->first_player;
num_winners = 1;

/* assign ranks to all players hands */
for (tmp_player = game->first_player; tmp_player != NULL;
        tmp_player = tmp_player->next) {
	/* If the player is not folded */
	if (tmp_player->hand[0] != -1) {
        /* sort cards */
        for(i=0;i<5;i++) {
		for(j=0;j<4;j++)
                {
                if(tmp_player->hand[j]%13 < tmp_player->hand[j+1]%13) {
                        temp=tmp_player->hand[j];
                        tmp_player->hand[j]=tmp_player->hand[j+1];
                        tmp_player->hand[j+1]=temp;
                        }
                }
        }
        /*      
        for (i = 0; i < 5; i++) {
		sprintf(text, "%s's hand[%d]mod 13 = %d\n",
                      tmp_player->user->name, i, tmp_player->hand[i]%13);
                      write_room(game->room, text);
                      } */
	  /* check for straight or straight flush */
	  /* Ace low 5432A */
	  if ((tmp_player->hand[0]%13 == (tmp_player->hand[1]%13) + 1) &&
                (tmp_player->hand[1]%13 == (tmp_player->hand[2]%13) + 1) &&
                (tmp_player->hand[2]%13 == (tmp_player->hand[3]%13) + 1) &&
                (tmp_player->hand[4]%13 == 12) && (tmp_player->hand[0]%13 == 0)) {
		tmp_player->rank = 5;
		/* arrange cards */
		swap_cards(tmp_player->hand, 0, 1);
		swap_cards(tmp_player->hand, 1, 2);
		swap_cards(tmp_player->hand, 2, 3);
		swap_cards(tmp_player->hand, 3, 4);
                }
           else { /* Ace high AKQJ10 or other straight */
		if ((tmp_player->hand[0]%13 == (tmp_player->hand[1]%13) + 1) &&
			(tmp_player->hand[1]%13 == (tmp_player->hand[2]%13) + 1) &&
			(tmp_player->hand[2]%13 == (tmp_player->hand[3]%13) + 1) &&
			(tmp_player->hand[3]%13 == (tmp_player->hand[4]%13) + 1)) {
                        tmp_player->rank = 5;
                        }        
	  }
	
	  /* check for flush */
	  if (((tmp_player->hand[0] < 13) &&
		   (tmp_player->hand[1] < 13) &&
		   (tmp_player->hand[2] < 13) &&
		   (tmp_player->hand[3] < 13) &&
		   (tmp_player->hand[4] < 13)) ||
		  (((tmp_player->hand[0] < 26) && (tmp_player->hand[0] > 12)) &&
		   ((tmp_player->hand[1] < 26) && (tmp_player->hand[1] > 12)) &&
		   ((tmp_player->hand[2] < 26) && (tmp_player->hand[2] > 12)) &&
		   ((tmp_player->hand[3] < 26) && (tmp_player->hand[3] > 12)) &&
		   ((tmp_player->hand[4] < 26) && (tmp_player->hand[4] > 12))) ||
		  (((tmp_player->hand[0] < 39) && (tmp_player->hand[0] > 25)) &&
		   ((tmp_player->hand[1] < 39) && (tmp_player->hand[1] > 25)) &&
		   ((tmp_player->hand[2] < 39) && (tmp_player->hand[2] > 25)) &&
		   ((tmp_player->hand[3] < 39) && (tmp_player->hand[3] > 25)) &&
		   ((tmp_player->hand[4] < 39) && (tmp_player->hand[4] > 25))) ||
		  (((tmp_player->hand[0] < 52) && (tmp_player->hand[0] > 38)) &&
		   ((tmp_player->hand[1] < 52) && (tmp_player->hand[1] > 38)) &&
		   ((tmp_player->hand[2] < 52) && (tmp_player->hand[2] > 38)) &&
		   ((tmp_player->hand[3] < 52) && (tmp_player->hand[3] > 38)) &&
		   ((tmp_player->hand[4] < 52) && (tmp_player->hand[4] > 38)))) {
                   /* We have a flush at least */
                   if (tmp_player->rank > 0) { 
                   /* We have a straight flush */
                   tmp_player->rank = 9;
                   }
              else {
		  /* We have a flush */
		  tmp_player->rank = 6;
                  }
	  }
  	  /* check for four of a kind */
	  if ((tmp_player->rank == 0) && 
		  ((tmp_player->hand[0]%13 == tmp_player->hand[1]%13) &&
		   (tmp_player->hand[1]%13 == tmp_player->hand[2]%13) &&
		   (tmp_player->hand[2]%13 == tmp_player->hand[3]%13))) {
		tmp_player->rank = 8;
	  }
	  if ((tmp_player->rank == 0) &&
		   ((tmp_player->hand[1]%13 == tmp_player->hand[2]%13) &&
			(tmp_player->hand[2]%13 == tmp_player->hand[3]%13) &&
			(tmp_player->hand[3]%13 == tmp_player->hand[4]%13))) {
		/* we have four of a kind */
		tmp_player->rank = 8;
		/* arrange cards */
		swap_cards(tmp_player->hand, 0, 4);
	  }
			   
	  /* check for three of a kind or full house */
	  if ((tmp_player->rank == 0) && 
                ((tmp_player->hand[1]%13 == tmp_player->hand[2]%13) &&
                (tmp_player->hand[2]%13 == tmp_player->hand[3]%13))) {
		/* we have three of a kind */
		tmp_player->rank = 4;
		/* arrange cards */
		swap_cards(tmp_player->hand, 0, 3);
	  }
	  if ((tmp_player->rank == 0) && 
                ((tmp_player->hand[0]%13 == tmp_player->hand[1]%13) &&
                (tmp_player->hand[1]%13 == tmp_player->hand[2]%13))) {
		if (tmp_player->hand[3]%13 == tmp_player->hand[4]%13) {
                /* we have a full house */
                tmp_player->rank = 7;
		} else {
                /* we have three of a kind */
                tmp_player->rank = 4;
		}
	  }
	  if ((tmp_player->rank == 0) && 
                ((tmp_player->hand[2]%13 == tmp_player->hand[3]%13) &&
                (tmp_player->hand[3]%13 == tmp_player->hand[4]%13))) {
		if (tmp_player->hand[0]%13 == tmp_player->hand[1]%13) {
                /* we have a full house */
                tmp_player->rank = 7;
                /* arrange cards */
                swap_cards(tmp_player->hand, 0, 3);
                swap_cards(tmp_player->hand, 1, 4);
		} else {
                /* we have three of a kind */
                tmp_player->rank = 4;
                /* arrange cards */
                swap_cards(tmp_player->hand, 0, 3);
                swap_cards(tmp_player->hand, 1, 4);
		}
	  }

	  /* check for two pair */
	  if ((tmp_player->rank == 0) && 
		  ((tmp_player->hand[0]%13 == tmp_player->hand[1]%13) &&
		   (tmp_player->hand[2]%13 == tmp_player->hand[3]%13))) {
          /* we have two pair */
          tmp_player->rank = 3;
	  }
	  /* check for two pair */
	  if ((tmp_player->rank == 0) && 
                ((tmp_player->hand[0]%13 == tmp_player->hand[1]%13) &&
                (tmp_player->hand[3]%13 == tmp_player->hand[4]%13))) {
		/* we have two pair */
		tmp_player->rank = 3;
		/* arrange cards */
		swap_cards(tmp_player->hand, 2, 4);
	  }		
	  /* check for two pair */
	  if ((tmp_player->rank == 0) && 
                ((tmp_player->hand[1]%13 == tmp_player->hand[2]%13) &&
                (tmp_player->hand[3]%13 == tmp_player->hand[4]%13))) {
		/* We have two pair */
		tmp_player->rank = 3;
		/* arrange cards */
		swap_cards(tmp_player->hand, 0, 2);
		swap_cards(tmp_player->hand, 2, 4);
	  }
	  /* check for one pair */
	  if ((tmp_player->rank == 0) && 
                (tmp_player->hand[0]%13 == tmp_player->hand[1]%13)) {
		/* we have a pair */
		tmp_player->rank = 2;
	  }
	  /* check for one pair */
	  if ((tmp_player->rank == 0) && 
                (tmp_player->hand[1]%13 == tmp_player->hand[2]%13)) {
		/* we have a pair */
		tmp_player->rank = 2;
		/* arrange cards */
		swap_cards(tmp_player->hand, 0, 2);
	  }
	  /* check for one pair */
	  if ((tmp_player->rank == 0) && 
                (tmp_player->hand[2]%13 == tmp_player->hand[3]%13)) {
		/* we have a pair */
		tmp_player->rank = 2;
		/* arrange cards */
		swap_cards(tmp_player->hand, 0, 2);
		swap_cards(tmp_player->hand, 1, 3);
	  }
	  /* check for one pair */
	  if ((tmp_player->rank == 0) && 
                (tmp_player->hand[3]%13 == tmp_player->hand[4]%13)) {
		/* We have a pair */
		tmp_player->rank = 2;
		/* arrange cards */
		swap_cards(tmp_player->hand, 0, 2);
		swap_cards(tmp_player->hand, 1, 3);
		swap_cards(tmp_player->hand, 0, 4);
	  }
	  if (tmp_player->rank == 0) {
		/* We must have a high card */
		tmp_player->rank = 1;
                }
        }
}

/* show all hands that are in */
for (tmp_player = game->first_player; tmp_player != NULL;
	   tmp_player = tmp_player->next) {
           if (tmp_player->rank > 0) {
           /* get max rank */
           if (tmp_player->rank > winners[0]->rank) { 
		winners[0] = tmp_player;
		num_winners = 1;
                } 
           if ((tmp_player->rank == winners[0]->rank) &&
                (tmp_player != winners[0])){
		for (i = 0; i < 5; i++) {
                if (tmp_player->hand[i]%13 > winners[0]->hand[i]%13) {
			winners[0] = tmp_player;
			num_winners = 1;
			break;
                        }
                        else if (tmp_player->hand[i]%13 < winners[0]->hand[i]%13) {
			break;
                        } else if (i == 4) {
			/* we have a tie */
			winners[num_winners] = tmp_player;
			num_winners++;
                        }
		}
	  }		  
	room_print_hand(tmp_player->user, tmp_player->hand);
	switch (tmp_player->rank) {
        case 1: sprintf(rtext,"a high card."); break;
        case 2: sprintf(rtext,"a pair."); break;
        case 3: sprintf(rtext,"two pair."); break;
        case 4: sprintf(rtext,"three of a kind."); break;
        case 5: sprintf(rtext,"a straight."); break;
        case 6: sprintf(rtext,"a flush."); break;
        case 7: sprintf(rtext,"a full house."); break;
        case 8: sprintf(rtext,"four of a kind."); break;
        case 9: sprintf(rtext,"a straight flush."); break;
	}
        sprintf(text,"~OL~FW-> ~OL~FG%s~FT has~FG %s\n",tmp_player->user->name,rtext);
	write_room(game->room, text);
	record(game->room,text);
	}
  }
/*sprintf(text, "num_winners = %d\n", num_winners);
  write_room(game->room, text); */
/* divide the loot */
/* curr_player == the player who was called */
for (i = 0; i < num_winners; i++) {
	temp = (game->pot / (num_winners * 5));
	loot = 5 * temp;
        /*sprintf(text, "temp = %d\n", temp);
        write_room(game->room, text);*/
/* player called gets remainder */
if (winners[i] == game->curr_player) {
        loot += game->pot%(num_winners*5);
	}
        winners[i]->hist->total += loot;
        sprintf(text,"~OL~FW-> ~FTYou win~FG $%d~FW!!!\n", loot);
	write_user(winners[i]->user, text);
        sprintf(text,"~OL~FW-> ~FG%s~FT wins~FG $%d~FT!!!\n",winners[i]->user->name,loot);
	write_room_except(game->room, text, winners[i]->user);
	record(game->room, text);
        }
sort_hist();
game->pot = 0;
game->bet = 0;
game->state = 0;  /* reset and deal cards */
for (tmp_player = game->first_player; tmp_player != NULL;
        tmp_player = tmp_player->next) {
	/* clear players' hands */
	tmp_player->hand[0] = -1;
        }

/* pass the deal if we haven't already */
if (!game->newdealer)
	pass_the_deal(game);
game->curr_player = game->dealer;
sprintf(text,"~OL~FW-> ~FGIt's your turn to deal.\n");
        write_user(game->dealer->user, text);
        sprintf(text,"~OL~FW-> ~OL~FTIt's~FG %s's~FT turn to deal.\n",game->dealer->user->name);
        write_room_except(game->room, text, game->dealer->user);
        record(game->room, text);
        }
  

/*** swap_cards ***/
void swap_cards(int hand[],int c1,int c2)
{
int tmp;
tmp = hand[c1];
hand[c1] = hand[c2];
hand[c2] = tmp;
}

void magic_poker(UR_OBJECT user)
{
user->pop->hand[0] = atoi(word[1]);
user->pop->hand[1] = atoi(word[2]);
user->pop->hand[2] = atoi(word[3]);
user->pop->hand[3] = atoi(word[4]);
user->pop->hand[4] = atoi(word[5]);
sprintf(text,"~OL~FG%s~FT fiddles with the cards.\n",user->name);
write_room(user->room,text);
}

/*** Show the poker players ***/
void show_po_players(UR_OBJECT user)
{
struct po_game *game;
struct po_player *player;
char turn_text[80];
char text2[80];
if (word_count < 2) {
	if (user->pop == NULL) {
        if ((game=get_po_game_here(user->room))==NULL) {
                write_user(user,"~OLWhich poker game are you interested in?\n");
		return;
                }
/* else game = the game in this room */
	}
   else {
        game = user->pop->game;
	}
  } else {
        if ((game=get_po_game(word[1]))==NULL) {
                write_user(user,"~OLNo poker games by that name are being played.\n");
                return;
                }
}

/* show who is in */
/* fix opening bet bug */
/* fix bug in fold and quit */

sprintf(text,"\n~FM-~OL=~FR[ ~FYInfo For Game ~FT%s ~FR]~FM=~RS~FM-\n",game->name);
write_user(user, text);
write_user(user,"~OL~FTName         ~FW:  ~FBState  ~FW: ~FYChips            \n");
write_user(user,"~FG-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
        switch (game->state) {
        case 0: sprintf(turn_text,"~OL<-- ~FM*deal*~RS\n"); break;
        case 1: sprintf(turn_text,"~OL<-- ~FM*bet* (1st Round)~RS\n"); break;
        case 2: sprintf(turn_text,"~OL<-- ~FM*discard*~RS\n"); break;
        case 3: sprintf(turn_text,"~OL<-- ~FM*bet* (2nd Round)~RS\n"); break;
        case 4: sprintf(turn_text,"~OL<--\n~RS"); break;
        } 
for (player = game->first_player; player != NULL; player = player->next) {
	sprintf(text, "%-12s : ", player->user->name);
	if (game->state == 0) {
        strcat(text,"~FR~OLwaiting~RS : ");
	} else {
        if (player->hand[0] == -1) {
                strcat(text," ~FR~OLfolded~RS : ");
	  } else {
                strcat(text,"~FG~OLplaying~RS : ");
                }
	}
        sprintf(text2, "$%-6d ",player->hist->total);
	strcat(text, text2);
	if (game->curr_player == player) {
        strcat(text, turn_text);
	} else {
        strcat(text, "\n");
	}
	write_user(user, text);
}
write_user(user,"~FG-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
if ((game->state == 1)||(game->state == 3)) {
	if (user->pop != NULL) {
        sprintf(text,"~OL-> You have put~FG $%d into the pot during this betting round.\n",user->pop->putin);
	write_user(user, text); }
        sprintf(text,"~OL-> The current bet is~FG $%d.\n",game->bet);
	write_user(user, text);
        }
        sprintf(text,"~OL-> The current pot is~FG $%d.\n",game->pot);
        write_user(user, text);
        }

void check_hist(UR_OBJECT user)
{
static int done=0;

if (mesg_check_hour==thour && mesg_check_min==tmin) { if (done) return; }
else { done=0; return; }
done=1;
save_hist();
if (user) write_user(user,"~OL~FGSaved poker rankings.\n");
}

/************************************************************************/
/* My random function **/
int myRand(int max)
{
int n;
int mask;
/* Mask out as many bits as possible */
for (mask = 1; mask < max; mask *= 2);
mask -= 1;
/* Reroll until a number <= max is returned */
do {
        n = random()&mask;
        }
        while (n >= max);
        return(n);
        }

/*****************************************************************************/
/*                     MoeNUTS v1.50+ Tic Tac Toe v1.11                      */
/*****************************************************************************/

void tictac(UR_OBJECT user,char *inpstr)
{

UR_OBJECT u;
char temp_s[ARR_SIZE];
char *remove_first();
int move;

if (word_count<2) {
	write_user(user,"Usage   :  tictac [<user>] [<#>] [reset][show][status][say]]\n");
	write_user(user,"Examples:  tictac <user>   =  Challenge A User To Tic Tac Toe\n");
	write_user(user,"           tictac <#>      =  Place an 'X' in spot '#'\n");
	write_user(user,"           tictac reset    =  Reset Tic Tac Toe\n");
	write_user(user,"           tictac show     =  Redisplay Board\n");
	write_user(user,"           tictac status   =  Your Tic Tac Toe Status\n");
	write_user(user,"           tictac say      =  Speak To Your Opponent.\n");
	return;
	}
if (strstr(word[1],"reset")) {
	write_user(user,"~OL~FGResetting The Current Tic Tac Toe Game...\n");
	reset_tictac(user);
	if (user->opponent!=NULL) reset_tictac(user->opponent);
	return;
	}
if (strstr(word[1],"show")) {
	if (!user->opponent) {
		write_user(user,"~FRYou are not currently playing Tic Tac Toe!\n");
		return;
		}
	print_tic(user);
	return;
	}
if (strstr(word[1],"stat")) {
	write_user(user,"~OL~FM-----------------~FT[ ~FYYour Tic Tac Toe Statistics ~FT]~FM---------------\n\n");
	sprintf(text,"~FGYou won ~FY%d ~FGgames, Lost ~FY%d ~FGgames and had ~FY%d~FG tie games.\n\n",user->twin,user->tlose,user->tdraw);
	write_user(user,text);
	return;
	}
if (strstr(word[1],"say")) {
	if (!user->opponent) {
		write_user(user,"~OL~FRYou have to be playing a game of Tic Tac Toe!\n");
		return;
		}
	inpstr=remove_first(inpstr);
	if (!inpstr[0]) { write_user(user,"Say what to your opponent?\n"); return; }
	sprintf(text,"~OL~FW-> ~FGYou say to ~FT%s~RS~OL~FW: ~FB\"~RS%s~OL~FB\"\n",user->opponent->recap,inpstr);
	write_user(user,text);
	sprintf(text,"~OL~FW-> ~FT%s~RS~OL~FG says to you~FW: ~FB\"~RS%s~OL~FB\"\n",user->opponent->recap,inpstr);
	write_user(user->opponent,text);
	return;
	}
if (atoi(word[1])>9) {
	write_user(user,"~OL~FRThere are only nine spots on the Tic Tac Toe board!\n");
	return;
	}
if (!isdigit(word[1][0])) {
        u=get_user(word[1]);  /* Replaced get_opp. (get_opp = EVIL!) */
	if (u==NULL) {
       		write_user(user,notloggedon);
                return;
      		}
	if (u==user) {
		write_user(user,"~OL~FMYou cannot play Tic Tac Toe With yourself!\n");
		return;
		}
        reset_tictac(user);
    	user->opponent=u;
    	if (user->opponent->opponent) {
       		if (user->opponent->opponent!=user) {
                        write_user(user, "Sorry, that person is already playing Tic Tac Toe.\n");
           		return;
          		}
               if (user->opponent->level<USER) {
                    write_user(user,"~OL~FRThat user doesn't have Tic Tac Toe!\n");
           		return;
          		}
               sprintf(temp_s, "~OL~FG%s ~FMagrees to play Tic Tac Toe with you\n",user->name);
               write_user(user->opponent,temp_s);
               sprintf(temp_s, "~OL~FMYou agree to a game of Tic Tac Toe with ~FG%s\n",user->opponent->name);
       		write_user(user,temp_s);
               sprintf(temp_s,"~OL~FR%s ~FTstarts playing Tic Tac Toe with ~FR%s\n",user->name,user->opponent->name);
       		write_room(user->room,temp_s);
       		print_tic(user); print_tic(user->opponent);
       		return;
      		}
    	else {
               sprintf(temp_s, "~OL~FG%s ~RS~OL~FMwants to play a game of Tic Tac Toe with you.\nYou can use '.tictac %s' to accept the game!\n",user->recap,user->name);
	       	write_user(user->opponent,temp_s);
               sprintf(temp_s,"~OL~FMYou ask ~FG%s ~FMto play a game of Tic Tac Toe.\n",user->opponent->recap);
       		write_user(user,temp_s);
       		return;
      		}
   	}   
if (!user->opponent) {
	write_user(user,"~OL~FRYou are not playing Tic Tac Toe with anyone!\n");
    	return;
	}
if (user->opponent->opponent!=user) {
    	write_user(user,"~OL~FRThat user has not accepted yet.\n");
    	return;
   	}
if (!strcmp(user->array,"000000000") && !user->opponent->first) {
    	user->first=1;
   	}
move=word[1][0]-'0';
if (legal_tic(user->array,move,user->first)) {
    	user->array[move-1] = 1;
    	user->opponent->array[move-1] = 2;
     print_tic(user);
    	print_tic(user->opponent);
   	}
else {
    	write_user(user,"~OL~FRThat is an illegal move!\n");
	write_user(user,"~OL~FMIf this is your first move, try .tictac reset and re-start the game!\n");
    	return;
   	}   
if (!win_tic(user->array)) return;
if (win_tic(user->array)==1) {
        sprintf(temp_s, "~OL~FR%s ~RS~OL~FYhas beaten ~FR%s~RS~OL~FY at Tic Tac Toe.\n",user->recap,user->opponent->recap);
        user->twin++;
        user->opponent->tlose++;
        }
else if (win_tic(user->array) == 2) {
        sprintf(temp_s, "~OL~FR%s ~RS~OL~FYhas beaten~FR %s ~RS~OL~FYat Tic Tac Toe.\n",user->opponent->recap, user->recap);
        user->opponent->twin++;
        user->tlose++;
        }
else {
        sprintf(temp_s,"~OL~FTIt's a draw between~FR %s ~RS~OL~FT~FTand ~FR%s~RS~OL~FT.\n",user->recap,user->opponent->recap);
        user->opponent->tdraw++; 
        user->tdraw++;
   	}
write_room(user->room, temp_s);
strcpy(user->array, "000000000");
strcpy(user->opponent->array,"000000000");
user->first=0;
user->opponent->first=0;
user->opponent->opponent=NULL;
user->opponent=NULL;
}

    
int legal_tic(char *array,int move,int first)
{
int count1=0,count2=0;
int i;
       
if (array[move-1]==1||array[move-1]==2) return 0;
for (i=0;i<9;i++) {
	if (array[i]==1) count1++;
	else if (array[i]==2) count2++;
	}
if (count1>count2) return 0;
if (first!=1) if (count1==count2) return 0; 
return 1;
}

int win_tic(char *array)
{
int i,j;
int person;
    
for (person=1;person<3;person++) {        
	for (i=0;i<3;i++) 
		for (j=0;j<3;j++) {
      			if (array[i*3+j]!=person) break;
          		if (j==2) return person;
         		}
	for (i=0;i<3;i++) 
		for (j=0;j<3;j++) {
			if (array[j*3+i]!=person) break;
			if (j==2) return person;
			}  
	if (array[0]==person&&array[4]==person&&array[8]==person)
		return person;
	if (array[2]==person&&array[4]==person&&array[6]==person)
		return person;
	}
for (i=0,j=0;i<9;i++) {
	if (array[i]==1||array[i]==2) j++;
	}
if (j==9) return 3;
return 0;
}

void print_tic(UR_OBJECT user)
{          
char temp_s[ARR_SIZE];
char array[10];
int i;
          
for(i=0;i<9;i++) {
     if (user->array[i]==1) array[i]='X';
     else if (user->array[i]==2) array[i]='O';
     else array[i]=' ';
     }
if (user->high_ascii) {
     write_user(user,"\n");
     write_user(user,"~BM~OL~FB浜様様様様冤様様様様冤様様様様�~RS\n");
     write_user(user,"~BM~OL~FB�   ~FM�~FG1~FM�~FB   �   ~FM�~FG2~FM�~FB   �   ~FM�~FG3~FM�~FB   �~RS\n");
     write_user(user,"~BM~OL~FB�         �         �         �~RS\n");
     sprintf(temp_s, "~BM~OL~FB�    ~FY%c~FB    �    ~FY%c~FB    �    ~OL~FY%c~FB    �~RS    ~OL~FTYour Opponent Is~FW: ~FY%s ~FM(~FYO~FM)\n",array[0],array[1],array[2],user->opponent->name);
     write_user(user,temp_s);
     sprintf(temp_s, "~BM~OL~FB�         �         �         �~RS    ~OL~FTYour  Wins~FW: ~FG%d~FT, Loses~FW: ~FG%d~FT, Draws~FW: ~FG%d\n",user->twin,user->tlose,user->tdraw);
     write_user(user,temp_s);
     sprintf(temp_s, "~BM~OL~FB把陳陳陳陳田陳陳陳陳田陳陳陳陳�~RS    ~OL~FTTheir Wins~FW: ~FG%d~FT, Loses~FW: ~FG%d~FT, Draws~FW: ~FG%d\n",user->opponent->twin,user->opponent->tlose,user->opponent->tdraw);
     write_user(user,temp_s);
     write_user(user,"~BM~OL~FB�   ~FM�~FG4~FM�~FB   �   ~FM�~FG5~FM�~FB   �   ~FM�~FG6~FM�~FB   �~RS\n");
     write_user(user,"~BM~OL~FB�         �         �         �~RS\n");
     sprintf(temp_s, "~BM~OL~FB�    ~FY%c~FB    �    ~FY%c~FB    �    ~FY%c~FB    �~RS\n",array[3],array[4],array[5]);
     write_user(user,temp_s);
     write_user(user,"~BM~OL~FB�         �         �         �~RS\n");
     write_user(user,"~BM~OL~FB把陳陳陳陳田陳陳陳陳田陳陳陳陳�~RS\n");
     write_user(user,"~BM~OL~FB�   ~FM�~FG7~FM�~FB   �   ~FM�~FG8~FM�~FB   �   ~FM�~FG9~FM�~FB   �~RS\n");
     write_user(user,"~BM~OL~FB�         �         �         �~RS\n");
     sprintf(temp_s, "~BM~OL~FB�    ~FY%c~FB    �    ~FY%c~FB    �    ~FY%c~FB    �~RS\n",array[6],array[7],array[8]);
     write_user(user,temp_s);
     write_user(user,"~BM~OL~FB�         �         �         �~RS\n");
     write_user(user,"~BM~OL~FB藩様様様様詫様様様様詫様様様様�~RS\n");
     write_user(user,"\n");
     }
else {
     write_user(user,"\n");
     write_user(user,"~BM~OL~FB.=============================.~RS\n");
     write_user(user,"~BM~OL~FB!   ~FM<~FG1~FM>~FB   |   ~FM<~FG2~FM>~FB   |   ~FM<~FG3~FM>~FB   !~RS\n");
     write_user(user,"~BM~OL~FB!         |         |         !~RS\n");
     sprintf(temp_s, "~BM~OL~FB!    ~FY%c~FB    |    ~FY%c~FB    |    ~OL~FY%c~FB    !~RS    ~OL~FTYour Opponent Is~FW: ~FY%s ~FM(~FYO~FM)\n",array[0],array[1],array[2],user->opponent->name);
     write_user(user,temp_s);
     sprintf(temp_s, "~BM~OL~FB!         |         |         !~RS    ~OL~FTYour  Wins~FW: ~FG%3d~FT, Loses~FW: ~FG%3d~FT, Draws~FW: ~FG%3d\n",user->twin,user->tlose,user->tdraw);
     write_user(user,temp_s);
     sprintf(temp_s, "~BM~OL~FB!---------+---------+---------!~RS    ~OL~FTTheir Wins~FW: ~FG%3d~FT, Loses~FW: ~FG%3d~FT, Draws~FW: ~FG%3d\n",user->opponent->twin,user->opponent->tlose,user->opponent->tdraw);
     write_user(user,temp_s);
     write_user(user,"~BM~OL~FB!   ~FM<~FG4~FM>~FB   |   ~FM<~FG5~FM>~FB   |   ~FM<~FG6~FM>~FB   !~RS\n");
     write_user(user,"~BM~OL~FB!         |         |         !~RS\n");
     sprintf(temp_s, "~BM~OL~FB!    ~FY%c~FB    |    ~FY%c~FB    |    ~FY%c~FB    !~RS\n",array[3],array[4],array[5]);
     write_user(user,temp_s);
     write_user(user,"~BM~OL~FB!         |         |         !~RS\n");
     write_user(user,"~BM~OL~FB!---------+---------+---------!~RS\n");
     write_user(user,"~BM~OL~FB!   ~FM<~FG7~FM>~FB   |   ~FM<~FG8~FM>~FB   |   ~FM<~FG9~FM>~FB   |~RS\n");
     write_user(user,"~BM~OL~FB!         |         |         |~RS\n");
     sprintf(temp_s, "~BM~OL~FB!    ~FY%c~FB    |    ~FY%c~FB    |    ~FY%c~FB    |~RS\n",array[6],array[7],array[8]);
     write_user(user,temp_s);
     write_user(user,"~BM~OL~FB!         |         |         |~RS\n");
     write_user(user,"~BM~OL~FB+=============================+~RS\n");
     write_user(user,"\n");
     }
}

void reset_tictac(UR_OBJECT user)
{          
user->opponent=0;
strcpy(user->array,"000000000");
user->first=0;
}

/****************************************************************************/
/*  Hangman For MoeNUTS v1.50+  Code based on code found in Amnuts v1.4.2!  */
/*  Original Code By Andy Collington, Moenuts Updates By Michael R. Irving  */
/****************************************************************************/

/* Hangman Graphics (* ASCII Version *) */
char *hanged[8]={
  "\n~FR,---.  \n~FR|   ~FR   ~OL~FM     Moenuts Ascii Hangman v~FT1.00\n~FR|~OL~FR           ~FGWord To Guess~FW:~FY %s\n~FR|~OL~FR           ~FBLetters used ~FW:~FT %s \n~FR|~OL       \n~FR+------\n",
  "\n~FR,---.  \n~FR|   ~FR!  ~OL~FM     Moenuts Ascii Hangman v~FT1.00\n~FR|~OL~FR           ~FGWord To Guess~FW:~FY %s\n~FR|~OL~FR           ~FBLetters used ~FW:~FT %s \n~FR|~OL       \n~FR+------\n",
  "\n~FR,---.  \n~FR|   ~FR!  ~OL~FM     Moenuts Ascii Hangman v~FT1.00\n~FR|~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR|~OL~FR           ~FBLetters used ~FW:~FT %s \n~FR|~OL       \n~FR+------\n",
  "\n~FR,---.  \n~FR|   ~FR!  ~OL~FM     Moenuts Ascii Hangman v~FT1.00\n~FR|~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR|~OL~FR   #       ~FBLetters used ~FW:~FT %s \n~FR|~OL       \n~FR+------\n",
  "\n~FR,---.  \n~FR|   ~FR!  ~OL~FM     Moenuts Ascii Hangman v~FT1.00\n~FR|~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR|~OL~FR  /#       ~FBLetters used ~FW:~FT %s \n~FR|~OL       \n~FR+------\n",
  "\n~FR,---.  \n~FR|   ~FR!  ~OL~FM     Moenuts Ascii Hangman v~FT1.00\n~FR|~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR|~OL~FR  /#\\      ~FBLetters used ~FW:~FT %s\n~FR|~OL       \n~FR+------\n",
  "\n~FR,---.  \n~FR|   ~FR!  ~OL~FM     Moenuts Ascii Hangman v~FT1.00\n~FR|~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR|~OL~FR  /#\\      ~FBLetters used ~FW:~FT %s\n~FR|~OL  /    \n~FR+------\n",
  "\n~FR,---.  \n~FR|   ~FR!  ~OL~FM     Moenuts Ascii Hangman v~FT1.00\n~FR|~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR|~OL~FR  /#\\      ~FBLetters used ~FW:~FT %s\n~FR|~OL  / \\ \n~FR+------\n"
};

/* Hangman Graphics (* ANSI Version *) */
char *hanged_ansi[8]={
  "\n~FR崢陳�  \n~FR�   ~FR   ~OL~FM     Moenuts Ansi Hangman ~FTv1.00\n~FR�~OL~FR           ~FGWord To Guess~FW:~FY %s\n~FR�~OL~FR           ~FBLetters used ~FW:~FT %s \n~FR�~OL       \n~FR伉陳陳�\n",
  "\n~FR崢陳�  \n~FR�   ~FR�  ~OL~FM     Moenuts Ansi Hangman ~FTv1.00\n~FR�~OL~FR           ~FGWord To Guess~FW:~FY %s\n~FR�~OL~FR           ~FBLetters used ~FW:~FT %s \n~FR�~OL       \n~FR伉陳陳�\n",
  "\n~FR崢陳�  \n~FR�   ~FR�  ~OL~FM     Moenuts Ansi Hangman ~FTv1.00\n~FR�~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR�~OL~FR           ~FBLetters used ~FW:~FT %s \n~FR�~OL       \n~FR伉陳陳�\n",
  "\n~FR崢陳�  \n~FR�   ~FR�  ~OL~FM     Moenuts Ansi Hangman ~FTv1.00\n~FR�~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR�~OL~FR   �       ~FBLetters used ~FW:~FT %s \n~FR�~OL       \n~FR伉陳陳�\n",
  "\n~FR崢陳�  \n~FR�   ~FR�  ~OL~FM     Moenuts Ansi Hangman ~FTv1.00\n~FR�~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR�~OL~FR  /�       ~FBLetters used ~FW:~FT %s \n~FR�~OL       \n~FR伉陳陳�\n",
  "\n~FR崢陳�  \n~FR�   ~FR�  ~OL~FM     Moenuts Ansi Hangman ~FTv1.00\n~FR�~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR�~OL~FR  /�\\      ~FBLetters used ~FW:~FT %s\n~FR�~OL       \n~FR伉陳陳�\n",
  "\n~FR崢陳�  \n~FR�   ~FR�  ~OL~FM     Moenuts Ansi Hangman ~FTv1.00\n~FR�~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR�~OL~FR  /�\\      ~FBLetters used ~FW:~FT %s\n~FR�~OL  /    \n~FR伉陳陳�\n",
  "\n~FR崢陳�  \n~FR�   ~FR�  ~OL~FM     Moenuts Ansi Hangman ~FTv1.00\n~FR�~OL~FR   O       ~FGWord To Guess~FW:~FY %s\n~FR�~OL~FR  /�\\      ~FBLetters used ~FW:~FT %s\n~FR�~OL  / \\ \n~FR伉陳陳�\n"
};

void play_hangman(UR_OBJECT user)
{
int i;
char *get_hang_word();

if (word_count<2) {
	if (user->high_ascii) write_user(user,"~OL~FMMoenuts Ansi Hangman!\nUsage: hangman [play/restart/end/status]\n");
	else write_user(user,"~OL~FMMoenuts Ascii Hangman!\nUsage: hangman [play/restart/end/status]\n");
  	return;
	}
srand(time(0));
strtolower(word[1]);
i=0;
if (!strncmp("stat",word[1],4)) {
  if (user->hang_stage==-1) {
    write_user(user,"~OL~FRYou're not currently playing Hangman!\n");
    return;
    }
  write_user(user,"~OL~FGYour current hangman statistics:\n");
  if (strlen(user->hang_guess)<1) {
     if (user->high_ascii) sprintf(text,hanged_ansi[user->hang_stage],user->hang_word_show,"None yet!");
     else sprintf(text,hanged[user->hang_stage],user->hang_word_show,"None yet!");
     }
  else {
     if (user->high_ascii) sprintf(text,hanged_ansi[user->hang_stage],user->hang_word_show,user->hang_guess);
     else sprintf(text,hanged[user->hang_stage],user->hang_word_show,user->hang_guess);
     }
  write_user(user,text);
  write_user(user,"\n");
  return;
  }
if (!strcmp("end",word[1])) {
  if (user->hang_stage==-1) {
    write_user(user,"~OL~FRYou have to start a game before you can end it!\n");
    return;
    }
  user->hang_stage=-1;
  user->hang_word[0]='\0';
  user->hang_word_show[0]='\0';
  user->hang_guess[0]='\0';
  write_user(user,"~OL~FMYour current game of hangman has eneded.\n");
  return;
  }
if (!strcmp("play",word[1])) {
  if (user->hang_stage>-1) {
    write_user(user,"~OL~FRYou already have a game in progress!\n~OL~FRYou have to end this one before you can start over!\n");
    return;
    }
  get_hang_word(user->hang_word);
  strcpy(user->hang_word_show,user->hang_word);
  for (i=0;i<strlen(user->hang_word_show);++i) user->hang_word_show[i]='-';
  user->hang_stage=0;
  write_user(user,"~OL~FGYour current hangman statistics:\n\n");
  if (user->high_ascii) sprintf(text,hanged_ansi[user->hang_stage],user->hang_word_show,"None yet!");
  else sprintf(text,hanged[user->hang_stage],user->hang_word_show,"None yet!");
  write_user(user,text);
  return;
  }
if (!strncmp("rest",word[1],4)) {
  if (user->hang_stage!=-1) {
	write_user(user,"~OL~FMYour Current Game of Hangman Has Eneded...\n");
	write_user(user,"~OL~FTStarting A New Game Of Hangman...\n");
	}
  else write_user(user,"~OL~FGYou weren't currently playing...Starting a new game of hangman...\n");
  /* Clear The Current Game Stats */
  user->hang_stage=-1;
  user->hang_word[0]='\0';
  user->hang_word_show[0]='\0';
  user->hang_guess[0]='\0';
  /* Start A New Game */
  get_hang_word(user->hang_word);
  strcpy(user->hang_word_show,user->hang_word);
  for (i=0;i<strlen(user->hang_word_show);++i) user->hang_word_show[i]='-';
  user->hang_stage=0;
  write_user(user,"Your current hangman game stats:\n\n");
  if (user->high_ascii) sprintf(text,hanged_ansi[user->hang_stage],user->hang_word_show,"None yet!");
  else sprintf(text,hanged[user->hang_stage],user->hang_word_show,"None yet!");
  write_user(user,text);
  return;
  }
if (user->high_ascii) write_user(user,"~OL~FMMoenuts Ansi Hangman!\nUsage: hangman [play/restart/end/status]\n");
else write_user(user,"~OL~FMMoenuts Ascii Hangman!\nUsage: hangman [play/restart/end/status]\n");
}

/* returns a word from a list for hangman.  this will save loading words
   into memory, and the list could be updated as and when you feel like it */
char *get_hang_word(char *aword)
{
char filename[80];
FILE *fp;
int lines,cnt,i;

lines=cnt=i=0;
sprintf(filename,"%s/%s",MISCFILES,HANGDICT);
lines=count_lines(filename);
srand(time(0));
cnt=rand()%lines;

if (!(fp=fopen(filename,"r"))) return("moenuts");
fscanf(fp,"%s\n",aword);
while (!feof(fp)) {
  if (i==cnt) {
    fclose(fp);
    return aword;
    }
  ++i;
  fscanf(fp,"%s\n",aword);
  }
fclose(fp);
/* if no word was found, just return a generic word */
return("superman");
}

/* counts how many lines are in a file */
int count_lines(char *filename)
{
int i,c;
FILE *fp;

i=0;
if (!(fp=fopen(filename,"r"))) return i;
c=getc(fp);
while (!feof(fp)) {
  if (c=='\n') i++;
  c=getc(fp);
  }
fclose(fp);
return i;
}

/* Lets a user guess a letter for hangman */
void guess_hangman(UR_OBJECT user)
{
int count,i,blanks;

count=blanks=i=0;
if (word_count<2) {
  write_user(user,"Usage: guess <letter>\n");
  return;
  }
if (user->hang_stage==-1) {
  write_user(user,"~OL~FRYou're not playing hangman at the moment!\n~OL~FRTry: ~FM.hangman start\n");
  return;
  }
if (strlen(word[1])>1) {
  write_user(user,"~OL~FRHey!! Hey!! Hey!! ~FMOne letter at a time please!!\n");
  return;
  }
if (atoi(word[1])>0 || word[1][0]=='0') {
  write_user(user,"~OL~FRHey!  Guess 'LETTERS'! Not 'NUMBERS'! :)\n");
  return;
  }
strtolower(word[1]);
if (strstr(user->hang_guess,word[1])) {
  user->hang_stage++;
  write_user(user,"~OL~FRYou have already guessed that letter!  ~FMAnd you know what that means...\n\n");
  if (user->high_ascii) sprintf(text,hanged_ansi[user->hang_stage],user->hang_word_show,user->hang_guess);
  else sprintf(text,hanged[user->hang_stage],user->hang_word_show,user->hang_guess);
  write_user(user,text);
  if (user->hang_stage>=7) {
    write_user(user,"~FM-~OL=~FR[ ~FY~LISNAP ~RS~OL~FR]~FM=~RS~FM-  ~OL~FTThe snap of your kneck is heard as you are hanged!\n~OL~FMYou did not guess the word and have died...\n\n");
    user->hang_stage=-1;
    user->hang_word[0]='\0';
    user->hang_word_show[0]='\0';
    user->hang_guess[0]='\0';
    }
  write_user(user,"\n");
  return;
  }
for (i=0;i<strlen(user->hang_word);++i) {
  if (user->hang_word[i]==word[1][0]) {
    user->hang_word_show[i]=user->hang_word[i];
    ++count;
    }
  if (user->hang_word_show[i]=='-') ++blanks;
  }
strcat(user->hang_guess,word[1]);
if (!count) {
  user->hang_stage++;
  write_user(user,"~OL~FROoh, Tough luck!  ~FMThat letter isn't in the word!\n~OL~FRAnd you know what that means...\n");
  if (user->high_ascii) sprintf(text,hanged_ansi[user->hang_stage],user->hang_word_show,user->hang_guess);
  else sprintf(text,hanged[user->hang_stage],user->hang_word_show,user->hang_guess);
  write_user(user,text);
  if (user->hang_stage>=7) {
    write_user(user,"~FM-~OL=~FR[ ~FY~LISNAP ~RS~OL~FR]~FM=~RS~FM-  ~OL~FTThe snap of your kneck is heard as you are hanged!\n~OL~FMYou did not guess the word and have died...\n\n");
    user->hang_stage=-1;
    user->hang_word[0]='\0';
    user->hang_word_show[0]='\0';
    user->hang_guess[0]='\0';
    }
  write_user(user,"\n");
  return;
  }
if (count==1) sprintf(text,"~OL~FYWooHoo!  ~FGThere's 1 ~FB\"~FT%s~FB\"~FG in the word!\n",word[1]);
else sprintf(text,"~OL~FYWooHoo!  ~FGThere's ~FM%d ~FB\"~FT%s~FB\"~FG's in the word!\n",count,word[1]);
write_user(user,text);
if (user->high_ascii) sprintf(text,hanged_ansi[user->hang_stage],user->hang_word_show,user->hang_guess);
else sprintf(text,hanged[user->hang_stage],user->hang_word_show,user->hang_guess);
write_user(user,text);
if (!blanks) {
  write_user(user,"~FG~OLCongratulations! ~FB-~FM=~FT*~FM=~FB- ~FYYou've escaped death and guessed the word!\n");
  user->hang_stage=-1;
  user->hang_word[0]='\0';
  user->hang_word_show[0]='\0';
  user->hang_guess[0]='\0';
  }
}

void reset_hangman(UR_OBJECT user)
{
  user->hang_stage=-1;
  user->hang_word[0]='\0';
  user->hang_word_show[0]='\0';
  user->hang_guess[0]='\0';
}
/***************************************************************************/
/******************************* End Of Hangman ****************************/
/***************************************************************************/
