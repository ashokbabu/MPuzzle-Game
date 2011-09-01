#include <math.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gtk/gtkstyle.h>
#include <libosso.h>
#include "mpuzzle.h"

static max_undos_supported_count=0;
static int no_of_valid_undos=0;
GtkWidget *undo_button;
int alist[15];
/*
 * Deletes the last node in the stack for restricting number of undo 
 */
void free_last_element_in_undo_stack( struct pos **head )
{
	struct pos *ptrCurrent = *head;
	while(ptrCurrent->link!=NULL){

		if((ptrCurrent->link)->link==NULL){								
			struct pos *prtDelete = ptrCurrent->link;
			free(prtDelete);
			ptrCurrent->link = NULL; 
		}
		else{			
			ptrCurrent = ptrCurrent->link;				
		}
	}	

}
/*
 * Deletes all the elements present in the stack whenever new puzzle is generated
 */
void DeleteAllElements_in_undo_stack( struct pos **head )
{
	struct pos *deleteMe = *head;
	while(deleteMe)	{
		struct pos *next = deleteMe->link;
		free(deleteMe);
		deleteMe = next;
	}
	*head = NULL;
}



int main (int argc, char *argv[])
{
	osso_context_t* ctx = NULL;
	gtk_init (&argc, &argv);
	main_window = create_main_window ();
	ctx = osso_initialize("org.maemo.mpuzzle", "1.7", TRUE, NULL);
	if (ctx == NULL) {
		g_print("Failed to init LibOSSO\n");
		return 0;
	}
	g_print("LibOSSO Init done\n");
				
	gtk_widget_show_all (main_window);
	gtk_main ();
	return 0;
}

/*
 * Verify whether valid move or not 
 */
gboolean is_legal_slide (gint current_pos, gint vacant_pos)
{
	int current_x_pos, current_y_pos, vacant_x_pos, vacant_y_pos;
	int a, b;
	
	vacant_x_pos=vacant_pos%4 ; vacant_y_pos=(int)(vacant_pos/4);
	current_x_pos=current_pos%4 ; current_y_pos=(int)(current_pos/4);

	a=abs(vacant_x_pos - current_x_pos);
	b=abs(vacant_y_pos - current_y_pos);

	if ( ( (a==1 && b==0) || (a==0 && b==1) ) && !(a==1 && b==1) )  
		 return TRUE;
	else
		return FALSE;

}

/* 
 * Verity Whether game completed or not
 */  
gboolean has_player_won (void)
{
	int x;
	for (x=0; x<15; x++)	
		if (x!= (gint) gtk_object_get_data(GTK_OBJECT(buttons[x]),"current_pos"))
	 	    return FALSE;
	
	gtk_widget_set_sensitive(undo_button,FALSE);	
	return TRUE;
}

/* 
 * check the generated puzzle is solvable or not if not generate the random puzzle again
 */
gboolean store_sequence_check_is_puzzle_solvable(void)
{
	int y;
	int puzzle_seq[15];
	int sum=0,i,j;
	int blank_space_row_no;
	for(y=0;y<15;y++){
		puzzle_seq[y] = (gint) gtk_object_get_data(GTK_OBJECT(buttons[y]), "current_pos");
	}
	blank_space_row_no= (int)(vacant_pos/4)+1;
	for(i=0;i<14;i++) {
           for(j=i+1;j<=14;j++) {
	        if(puzzle_seq[i]>puzzle_seq[j])	{
	  	      sum += 1;  
		}
           }     
	}
       	if((sum + blank_space_row_no)%2 == 0){
		return TRUE;
	}	
      return FALSE;
}


 /* 
  * Action taken after button is clicked
  * & storting the positions in a stack for undo opertion
  */ 

void on_button_clicked (GtkWidget *button, gpointer user_data)
{
	int x_pos, y_pos;
	int current_pos;	
	gchar file_name[MAXSIZE];	
	struct pos *temp_undo_store_positions;
		
	temp_undo_store_positions = (struct pos *)(malloc(sizeof(struct pos)));
	current_pos = (gint) gtk_object_get_data (GTK_OBJECT(button), "current_pos");

			
	if ( is_legal_slide(current_pos, vacant_pos) ){
       		gtk_widget_set_sensitive(undo_button,TRUE); /* Activate undo button*/
		max_undos_supported_count++;	
			
		x_pos = vacant_pos%4; y_pos = (int)(vacant_pos/4);
		
		 if(temp_undo_store_positions){    
		     temp_undo_store_positions->attach_in_vac_pos = vacant_pos;
	             temp_undo_store_positions->link = undo_store_positions;
		     undo_store_positions = temp_undo_store_positions;		     
		 }else{
			 g_print("unable to allocate memory from heap for undo strcut");			 
			 gtk_main_quit();
			/* Quit application */
		 }

		no_of_valid_undos++;		
		gtk_container_remove (GTK_CONTAINER(table1), button);
		gtk_table_attach (GTK_TABLE (table1), 
			button, x_pos, x_pos + 1, y_pos, y_pos + 1,
			(GtkAttachOptions) GTK_FILL | GTK_EXPAND,
			(GtkAttachOptions) GTK_FILL | GTK_EXPAND,
			(GtkAttachOptions) 1, 1);

		gtk_object_set_data (GTK_OBJECT(button), "current_pos",(gpointer) vacant_pos);
		vacant_pos = current_pos;
		move_no++;
		
		if ( has_player_won() )	{
			gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, g_strdup_printf("Eureka You Won the Game with in %i Moves", move_no));		    	     gtk_widget_hide (table1);
			image = gtk_image_new ();
		        sprintf(file_name,"%swon.png",IMAGES_DIR);
		        gtk_image_set_from_file (GTK_IMAGE(image), file_name);
		        gtk_object_ref(GTK_OBJECT(table1));
		        gtk_container_remove (GTK_CONTAINER(alignment1), table1);
		        gtk_container_add (GTK_CONTAINER(alignment1), image);
		        gtk_widget_show (image);
		}else
			gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, g_strdup_printf("Move No.: %i", move_no));
	}
	else
		gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, "Wrong Move !");


	if(max_undos_supported_count > NO_OF_UNDOS_SUPPORTED){
		max_undos_supported_count=NO_OF_UNDOS_SUPPORTED;		
		free_last_element_in_undo_stack(&undo_store_positions);		
	}

}

GtkWidget* create_main_window (void)
{
  GtkWidget *main_window, *menubar1,*menuitem1,*menuitem1_menu,*new;
  GtkWidget *separatormenuitem1,*quit1,*menuitem4;
  GtkWidget *menuitem4_menu,*about1,*toolbar1;
  GtkWidget *new_button,*quit_button,*help_button;
/*  GtkWidget *undo_button;*/
  
  gint tmp_toolbar_icon_size, x, y, c=0;
  GtkAccelGroup *accel_group = gtk_accel_group_new ();
  GdkColor color = {0, 0xcc00, 0xec00, 0xff00};

  vacant_pos = 15; move_no=0;

  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (main_window), "MPuzzle-1.7");
  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (main_window), vbox1);

  menubar1 = gtk_menu_bar_new ();
  menuitem1 = gtk_menu_item_new_with_mnemonic ("_File");
  menuitem1_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem1), menuitem1_menu);
  new = gtk_image_menu_item_new_from_stock ("gtk-new", accel_group);
  separatormenuitem1 = gtk_separator_menu_item_new ();
  gtk_widget_set_sensitive (separatormenuitem1, FALSE);
  quit1 = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
  menuitem4 = gtk_menu_item_new_with_mnemonic ("_Help");
  menuitem4_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem4), menuitem4_menu);
  about1 = gtk_menu_item_new_with_mnemonic ("_About");

  gtk_box_pack_start (GTK_BOX (vbox1), menubar1, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (menubar1), menuitem1);
  gtk_container_add (GTK_CONTAINER (menuitem1_menu), new);
  gtk_container_add (GTK_CONTAINER (menuitem1_menu), separatormenuitem1);
  gtk_container_add (GTK_CONTAINER (menuitem1_menu), quit1);
  gtk_container_add (GTK_CONTAINER (menubar1), menuitem4);
  gtk_container_add (GTK_CONTAINER (menuitem4_menu), about1);

  toolbar1 = gtk_toolbar_new ();
  gtk_box_pack_start (GTK_BOX (vbox1), toolbar1, FALSE, FALSE, 0);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar1), GTK_TOOLBAR_BOTH);
  tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (toolbar1));

  new_button = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-new");
  gtk_container_add (GTK_CONTAINER (toolbar1), new_button);

  quit_button = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-quit");
  gtk_container_add (GTK_CONTAINER (toolbar1), quit_button);

  help_button = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-help");
  gtk_container_add (GTK_CONTAINER (toolbar1), help_button);

  undo_button = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-undo");
  gtk_container_add (GTK_CONTAINER (toolbar1), undo_button);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_box_pack_start (GTK_BOX (vbox1), alignment1, TRUE, TRUE, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 20, 20, 40, 40);

  table1 = gtk_table_new (4, 4, TRUE);
  gtk_container_add (GTK_CONTAINER (alignment1), table1);
  gtk_widget_modify_bg(main_window, GTK_STATE_NORMAL, &color);

  gtk_widget_set_sensitive(undo_button,FALSE);
  
  for (x=0; x<15; x++) {
	  gchar *temp = g_strdup_printf ("%i", x+1);
	  buttons[x] = gtk_button_new_with_mnemonic (temp);
	  gtk_object_set_data (GTK_OBJECT(buttons[x]), "current_pos", (gpointer) x);
	  GdkColor dark = {0,0x6900,0xc600,0xff00};
	  GtkStyle * style = gtk_style_copy(buttons[x]->style);
	  style->bg[0] = dark;
	  style->bg[1] = dark;
	  style->bg[2] = dark;
	  style->bg[3] = dark;
	  style->bg[4] = dark;
	  gtk_widget_set_style(buttons[x], style);
	  g_free (temp);
  }
 
  for (x=0; x<4; x++)
  	  for (y=0; y<4; y++)
  		  if (!(x==3 && y==3))
			  gtk_table_attach (GTK_TABLE (table1), 
	  		  buttons[c++], y,y+1,x,x+1,
			  (GtkAttachOptions) GTK_FILL | GTK_EXPAND,
      			  (GtkAttachOptions) GTK_FILL | GTK_EXPAND,
      			  (GtkAttachOptions) 1, 1);
  
  for (x=0; x<15; x++)
	gtk_signal_connect(GTK_OBJECT(buttons[x]), "clicked", GTK_SIGNAL_FUNC(on_button_clicked), (gpointer)x);

  statusbar1 = gtk_statusbar_new ();
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(statusbar1), FALSE);
  gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, "Welcome to MARS Telecom");
  gtk_box_pack_start (GTK_BOX (vbox1), statusbar1, FALSE, FALSE, 0);

  g_signal_connect ((gpointer) main_window, "delete_event", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect ((gpointer) new, "activate", G_CALLBACK (on_new_activate), NULL);
  g_signal_connect ((gpointer) new_button, "clicked", G_CALLBACK (on_new_activate), NULL);
  g_signal_connect ((gpointer) quit1, "activate", G_CALLBACK (on_quit_confirm), NULL); 
  g_signal_connect ((gpointer) about1, "activate", G_CALLBACK (on_about1_activate), NULL);
  g_signal_connect ((gpointer) quit_button, "clicked", G_CALLBACK (on_quit_confirm), NULL);
  g_signal_connect ((gpointer) help_button, "clicked", G_CALLBACK (on_about1_activate), NULL);
  g_signal_connect (GTK_OBJECT(undo_button),"clicked", GTK_SIGNAL_FUNC(on_undo_activate),NULL);
	
  gtk_window_add_accel_group (GTK_WINDOW (main_window), accel_group);

/*  gtk_widget_grab_focus(new_button);
  gtk_button_clicked (new_button);

  gtk_widget_grab_focus(undo_button);
  gtk_button_clicked (undo_button);
*/

  return main_window;
  
}

void swap_buttons (int p, int q)
{
	gint p_pos = (gint) gtk_object_get_data (GTK_OBJECT(buttons[p]), "current_pos");
	gint q_pos = (gint) gtk_object_get_data (GTK_OBJECT(buttons[q]), "current_pos");
	int p_x_pos = p_pos%4; 
	int p_y_pos = (int)(p_pos/4);
	int q_x_pos = q_pos%4; 
	int q_y_pos = (int)(q_pos/4);

	gtk_object_ref (GTK_OBJECT(buttons[p]));
	gtk_object_ref (GTK_OBJECT(buttons[q]));
	gtk_container_remove (GTK_CONTAINER(table1), buttons[p]);
	gtk_container_remove (GTK_CONTAINER(table1), buttons[q]);

	gtk_table_attach (GTK_TABLE (table1), 
			buttons[p], q_x_pos, q_x_pos + 1, q_y_pos, q_y_pos + 1,
			(GtkAttachOptions) GTK_FILL | GTK_EXPAND,
			(GtkAttachOptions) GTK_FILL | GTK_EXPAND,
			(GtkAttachOptions) 1, 1);
	gtk_table_attach (GTK_TABLE (table1), 
			buttons[q], p_x_pos, p_x_pos + 1, p_y_pos, p_y_pos + 1,
			(GtkAttachOptions) GTK_FILL | GTK_EXPAND,
			(GtkAttachOptions) GTK_FILL | GTK_EXPAND,
			(GtkAttachOptions) 1, 1);
    	gtk_object_set_data (GTK_OBJECT(buttons[p]), "current_pos", (gpointer) q_pos);
	gtk_object_set_data (GTK_OBJECT(buttons[q]), "current_pos", (gpointer) p_pos);
}


void on_new_activate (GtkWidget *button,GtkMenuItem *menuitem, gpointer user_data)
{
	generate_puzzle();	
}

void generate_puzzle()
{
	int i;
	
	vacant_prev_pos = vacant_pos;
	vacant_pos=15;
	
	int y,x,k,movpos;
	DeleteAllElements_in_undo_stack(&undo_store_positions);

	gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, "Welcome to MARS Telecom Puzzle");
	move_no=0;

        if (GTK_IS_WIDGET(image) && GTK_IS_WIDGET(image->parent)) {
               gtk_object_ref (GTK_OBJECT(image));
               gtk_container_remove (GTK_CONTAINER(alignment1), image); 
               gtk_container_add (GTK_CONTAINER(alignment1), table1);
               gtk_widget_show (table1);
        }
	

	for(x=0;x<15;x++)
		alist[x]=x;
	
	gint row_pos,col_pos,req_pos;	
	int xy;
	for(i=0;i<500;i++){
		movpos = abs(g_rand_int ( g_rand_new () )) % 4;		

	        col_pos = vacant_pos%4;
	        row_pos = vacant_pos/4;
				
		switch(movpos)
		{
			case UP:
				if(is_move_correct(UP))	{
					req_pos = (((row_pos-1)*4) + col_pos);
					xy =  kvalue(req_pos);
					alist[xy]=vacant_pos;
					vacant_pos = req_pos;
				}
				break;			       
			case DOWN:
				if(is_move_correct(DOWN)){
					req_pos = (((row_pos+1)*4)+col_pos);
					xy =  kvalue(req_pos);
					alist[xy]=vacant_pos;
					vacant_pos = req_pos;
				}
				break;
			case LEFT:
				if(is_move_correct(LEFT)){
					req_pos = (row_pos*4)+(col_pos-1);
					xy =  kvalue(req_pos);
					alist[xy]=vacant_pos;
					vacant_pos = req_pos;
				}
				break;
			case RIGHT:
				if(is_move_correct(RIGHT)){
					req_pos = (row_pos*4)+(col_pos+1);
					xy =  kvalue(req_pos);
					alist[xy]=vacant_pos;
					vacant_pos = req_pos;
				}
				break;
		}

		
		
	}	
	

	for(k=0;k<15;k++)
		if(vacant_pos == (gint)gtk_object_get_data (GTK_OBJECT(buttons[k]), "current_pos"))
			break;	
	
       	y =vacant_prev_pos%4;
	x = (int) vacant_prev_pos/4;
	
	gtk_object_ref(GTK_OBJECT(table1));
	gtk_object_ref (GTK_OBJECT(buttons[k]));
	
	gtk_container_remove (GTK_CONTAINER(table1),buttons[k] );
	
	gtk_table_attach (GTK_TABLE (table1), 
			buttons[k], y,y+1,x,x+1,
			(GtkAttachOptions) GTK_FILL | GTK_EXPAND,
			(GtkAttachOptions) GTK_FILL | GTK_EXPAND,
			(GtkAttachOptions) 1, 1);
	gtk_object_set_data (GTK_OBJECT(buttons[k]), "current_pos", (gpointer) vacant_prev_pos);

	int k1,k2;
	
	for(i=0;i<16;i++){
		if(i!= vacant_pos){
			for (k1=0;k1<15;k1++)
				if(i ==  (gint)gtk_object_get_data (GTK_OBJECT(buttons[k1]), "current_pos"))
					break;			
			for (k2=0;k2<15;k2++)
				if( i == alist[k2])
					break;
			if(k1!=k2)
			       	swap_buttons(k1,k2);
			
		 }
	}



/* This piece of code only for Testing Purpose */
	 if(store_sequence_check_is_puzzle_solvable()) {
		 g_print("\n Solvable Puzzle");
		 gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, "Solvable Puzzle");
	 } else{
		 g_print("\n Unsolvable Puzzle");
		 gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, "Unsolvable Puzzle");
	 }
		
 
	
}


int kvalue(int req_pos){
	 int k;
	 for(k=0;k<15;k++)
		 if(alist[k]==req_pos)
			 break;
	 return k;
	 
}

gboolean is_move_correct(int x){

	gint col_pos,row_pos;
	col_pos = vacant_pos%4;
	row_pos = vacant_pos/4;
	switch(x){
		case UP:
			if(row_pos == 0)
				return FALSE;
			break;
			
		case DOWN:
			if(row_pos ==3)
 				return FALSE;
			break;
		case LEFT:
			if(col_pos ==0)
 				return FALSE;
			break;
		case RIGHT:
			if(col_pos ==3)
			       return FALSE;
			break;
	}
	return TRUE;
}



void up(){
	int k;
	gint col_pos,row_pos,req_pos;
	
	row_pos = vacant_pos/4;
	col_pos = vacant_pos%4;

	req_pos = (((row_pos-1)*4) + col_pos);
	
	for(k=0;k<15;k++)
		if(alist[k]==req_pos)
			break;
	alist[k]=vacant_pos;
	vacant_pos = req_pos;	
	
}

void down(){
	int k;
	gint col_pos,row_pos,req_pos;
	row_pos = vacant_pos/4;
	col_pos = vacant_pos%4;

	req_pos = (((row_pos+1)*4)+col_pos);
	
	for(k=0;k<15;k++)		
		if(alist[k]==req_pos)
			break;

	alist[k]=vacant_pos;
	vacant_pos = req_pos;	
	
}

void left(){
	int k;	
	gint col_pos,row_pos,req_pos;
	row_pos = vacant_pos/4;
	col_pos = vacant_pos%4;

	req_pos = (row_pos*4)+(col_pos-1);
	
	for(k=0;k<15;k++)
		if(alist[k]==req_pos)
			break;
	alist[k]=vacant_pos;
	vacant_pos = req_pos;	
	
}

void right(){ 
	int k;
	gint col_pos,row_pos,req_pos;
	row_pos = vacant_pos/4;
	col_pos = vacant_pos%4;

	req_pos = (row_pos*4)+ (col_pos+1);
	for(k=0;k<15;k++)
		if(alist[k]== req_pos)
			break;

	alist[k]= vacant_pos;
	vacant_pos = req_pos;	
	
}




/* 
 * Action taken when undo button is pressed
 */ 
void on_undo_activate(GtkWidget *button,gpointer user_data)
{
	if(max_undos_supported_count>0) {
		max_undos_supported_count--;
	}
	gchar file_name[MAXSIZE];
	struct pos *temp_undo_store_positions;
	gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, "Cannot Perform Undo ");
	if(max_undos_supported_count <= 0){
   	gtk_widget_set_sensitive(undo_button,FALSE);
	}
	
	int k=0;

    if(no_of_valid_undos!=0){
		
        if(undo_store_positions==NULL){
		gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, "Cannot do Undo");		
        }
        else{
		gint x_pos,y_pos;
		x_pos = vacant_pos%4; 
		y_pos = (int)(vacant_pos/4);
		for(k=0;k<15;k++){
			if((gint) gtk_object_get_data (GTK_OBJECT(buttons[k]), "current_pos") == (undo_store_positions->attach_in_vac_pos))
				break;
	}


	gtk_object_ref(GTK_OBJECT(table1));
	gtk_object_ref (GTK_OBJECT(buttons[k]));
	gtk_container_remove (GTK_CONTAINER(table1),buttons[k] );
	
        gtk_table_attach (GTK_TABLE (table1), buttons[k], 
			x_pos, x_pos + 1, y_pos, y_pos +1,
		 	(GtkAttachOptions) GTK_FILL | GTK_EXPAND, 
			(GtkAttachOptions) GTK_FILL | GTK_EXPAND,(GtkAttachOptions) 1, 1);

        gtk_object_set_data (GTK_OBJECT(buttons[k]), "current_pos", (gpointer) vacant_pos);
        
	vacant_pos = undo_store_positions->attach_in_vac_pos;
        move_no++;
	if ( has_player_won() ) {
		gtk_widget_set_sensitive(undo_button,FALSE);
		gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, g_strdup_printf("Eureka You Won the Game with in Moves : %i", move_no));
		gtk_widget_hide (table1);
		image = gtk_image_new ();
		sprintf(file_name,"%swon.png",IMAGES_DIR);
		gtk_image_set_from_file (GTK_IMAGE(image), file_name);
		gtk_object_ref(GTK_OBJECT(table1));
		gtk_container_remove (GTK_CONTAINER(alignment1), table1);
		gtk_container_add (GTK_CONTAINER(alignment1), image);
		gtk_widget_show (image);
		}else {
			gtk_statusbar_push (GTK_STATUSBAR(statusbar1), 0, g_strdup_printf("Move No.: %i", move_no));
		}
	temp_undo_store_positions = undo_store_positions;
       	undo_store_positions = undo_store_positions->link;
	free(temp_undo_store_positions);
	}	
	no_of_valid_undos -=1;
    } 
}


void on_about1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *dialog;
		
	dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW(main_window),
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_INFO,
                                        GTK_BUTTONS_OK,
							"<b>Mpuzzle</b>\n\nSimple puzzle game\nHow to Play?\nClick on the number to move.\nIf you are able to sort numbers from 1 to 15\nthen you Win the Game!\nAuthor: Ashok Chiruvella\nCompany: http://mars-india.com\nemail: ramulu.kambalapuram@mars-india.com");
	
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}


void on_quit_confirm (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
			"Are you Sure want to Quit the Game");
	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
			GTK_STOCK_YES, GTK_RESPONSE_YES,
			GTK_STOCK_NO, GTK_RESPONSE_NO,
			NULL);
	
	result = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	
	if (result == GTK_RESPONSE_YES)
		gtk_main_quit();
	
}

