/*
 * Copyright (c) 2000 Sasha Vasko <sasha at aftercode.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#undef LOCAL_DEBUG
#include "../../configure.h"

#include "../../include/asapp.h"
#include "../../include/afterstep.h"
#include "../../include/mystyle.h"
#include "../../include/parser.h"
#include "../../include/confdefs.h"
#include "../../include/balloon.h"

void          ProcessStatement (ConfigDef * config);
/*****************************************************************************
 *
 * This routine is responsible for reading and parsing the base.<bpp> config
 * file
 *
 ****************************************************************************/

#define WHARF_FOLDER_END 	"~Folder"

TermDef       WhevTerms[] = {
	{TF_NO_MYNAME_PREPENDING | TF_SYNTAX_TERMINATOR, "push", 4, TT_FILENAME, WHEV_PUSH_ID, NULL},
	{TF_NO_MYNAME_PREPENDING | TF_SYNTAX_TERMINATOR, "close_folder", 12, TT_FILENAME, WHEV_CLOSE_FOLDER_ID, NULL},
	{TF_NO_MYNAME_PREPENDING | TF_SYNTAX_TERMINATOR, "open_folder", 11, TT_FILENAME, WHEV_OPEN_FOLDER_ID, NULL},
	{TF_NO_MYNAME_PREPENDING | TF_SYNTAX_TERMINATOR, "close_main", 10, TT_FILENAME, WHEV_CLOSE_MAIN_ID, NULL},
	{TF_NO_MYNAME_PREPENDING | TF_SYNTAX_TERMINATOR, "open_main", 9, TT_FILENAME, WHEV_OPEN_MAIN_ID, NULL},
	{TF_NO_MYNAME_PREPENDING | TF_SYNTAX_TERMINATOR, "drop", 4, TT_FILENAME, WHEV_DROP_ID, NULL},
	{0, NULL, 0, 0, 0, NULL}
};

SyntaxDef     WhevSyntax = {
	'\0',
	'\n',
	WhevTerms,
	0,										   /* use default hash size */
    ' ',
	"",
	"\t",
	"Wharf sound definition",
	NULL,
    0
};

TermDef       WharfTerms[] = {
	{TF_SPECIAL_PROCESSING, "", 0, TT_SPECIAL, WHARF_Wharf_ID, &FuncSyntax},
	{TF_NO_MYNAME_PREPENDING | TF_SYNTAX_TERMINATOR, WHARF_FOLDER_END, 7, TT_FLAG, WHARF_FolderEnd_ID, NULL},
    {0, "Geometry", 8,          TT_GEOMETRY, WHARF_Geometry_ID, NULL},
    {0, "Rows", 4,              TT_UINTEGER, WHARF_Rows_ID, NULL},
    {0, "Columns", 7,           TT_UINTEGER, WHARF_Columns_ID, NULL},
    {0, "NoPush", 6,            TT_FLAG, WHARF_NoPush_ID, NULL},
    {0, "FullPush", 8,          TT_FLAG, WHARF_FullPush_ID, NULL},
    {0, "NoBorder", 8,          TT_FLAG, WHARF_NoBorder_ID, NULL},
    {0, "WithdrawStyle", 13,    TT_UINTEGER, WHARF_WithdrawStyle_ID, NULL},
/* the NoWithdraw option is undocumented, deprecated, and
 ** may be removed at Wharf's maintainer's discretion */
    {0, "NoWithdraw", 10,       TT_FLAG, WHARF_NoWithdraw_ID, NULL},
    {0, "ForceSize", 9,         TT_GEOMETRY, WHARF_ForceSize_ID, NULL},
/* TextureType, MaxColors, BgColor, TextureColor, and Pixmap are obsolete */
	{TF_OBSOLETE, "TextureType", 11, TT_UINTEGER, WHARF_TextureType_ID, NULL},
    {TF_OBSOLETE, "MaxColors", 9,    TT_UINTEGER, WHARF_MaxColors_ID, NULL},
    {TF_OBSOLETE, "BgColor", 7,      TT_COLOR, WHARF_BgColor_ID, NULL},
    {TF_OBSOLETE, "TextureColor", 12,TT_COLOR, WHARF_TextureColor_ID, NULL},
    {TF_OBSOLETE, "Pixmap", 6,       TT_FILENAME, WHARF_Pixmap_ID, NULL},
	{0, "AnimateStepsMain", 16, TT_UINTEGER, WHARF_AnimateStepsMain_ID, NULL},
    {0, "AnimateSteps", 12,     TT_UINTEGER, WHARF_AnimateSteps_ID, NULL},
    {0, "AnimateDelay", 12,     TT_UINTEGER, WHARF_AnimateDelay_ID, NULL},
    {0, "AnimateMain", 11,      TT_FLAG, WHARF_AnimateMain_ID, NULL},
    {0, "Animate", 7,           TT_FLAG, WHARF_Animate_ID, NULL},
    {0, "Sound", 5,             TT_FILENAME, WHARF_Sound_ID, &WhevSyntax},
    {0, "ShowLabel", 9,         TT_FLAG, WHARF_ShowLabel_ID, NULL},
    {0, "LabelLocation", 13,    TT_UINTEGER, WHARF_LabelLocation_ID, NULL},
    {0, "FlipLabel", 9,         TT_FLAG, WHARF_FlipLabel_ID, NULL},
    {0, "FitContents", 11,      TT_FLAG, WHARF_FitContents_ID, NULL},
    {0, "ShapeToContents", 15,  TT_FLAG, WHARF_ShapeToContents_ID, NULL},
    {0, "AlignContents", 13,    TT_UINTEGER, WHARF_AlignContents_ID, NULL},


/* now special cases that should be processed by it's own handlers */
	BALLOON_TERMS,
/* including MyStyles definitions processing */
	INCLUDE_MYSTYLE,
	{0, NULL, 0, 0, 0}
};

SyntaxDef     WharfSyntax = {
	'\n',
	'\0',
	WharfTerms,
    0,                                         /* use default hash size */
	' ',
	"",
	"",
	"Wharf configuration",
	NULL,
    0
};

flag_options_xref WharfFlags[] = {
	{WHARF_NO_PUSH, WHARF_NoPush_ID, 0},
    {WHARF_FULL_PUSH, 0, WHARF_FullPush_ID },
	{WHARF_NO_BORDER, WHARF_NoBorder_ID, 0},
	{WHARF_NO_WITHDRAW, WHARF_NoWithdraw_ID, 0},
	{WHARF_ANIMATE_MAIN, WHARF_AnimateMain_ID, 0},
	{WHARF_ANIMATE, WHARF_Animate_ID, 0},
    {WHARF_SHOW_LABEL, WHARF_ShowLabel_ID, 0},
    {WHARF_FLIP_LABEL, WHARF_FlipLabel_ID, 0},
    {WHARF_FIT_CONTENTS, WHARF_FitContents_ID, 0},
    {WHARF_SHAPE_TO_CONTENTS, WHARF_ShapeToContents_ID, 0 },
    {0, 0, 0}
};

WharfButton  *
CreateWharfButton ()
{
	WharfButton  *btn = (WharfButton *) safemalloc (sizeof (WharfButton));

	memset (btn, 0x00, sizeof (WharfButton));
	return btn;
}

void
DestroyWharfButton (WharfButton **pbtn)
{
	register int  i;
    WharfButton *btn = *pbtn ;

	if (btn == NULL)
		return;
    *pbtn = btn->next ;

    /* delete members */
	if (btn->title != NULL)
		free (btn->title);

	if (btn->icon != NULL)
	{
		for (i = 0; btn->icon[i] != NULL; i++)
			free (btn->icon[i]);
		free (btn->icon);
	}

	if (btn->function)
	{
		free_func_data (btn->function);
		free (btn->function);
	}

	while (btn->folder)
        DestroyWharfButton (&(btn->folder));

	free (btn);
}


WharfConfig  *
CreateWharfConfig ()
{
	WharfConfig  *config = (WharfConfig *) safemalloc (sizeof (WharfConfig));

	memset (config, 0x00, sizeof (WharfConfig));
	/* let's initialize Base config with some nice values: */
	config->geometry.flags = WidthValue | HeightValue;
	config->geometry.width = config->geometry.height = 64;
    config->withdraw_style = WITHDRAW_ON_ANY_BUTTON ;
    config->align_contents = ALIGN_CENTER ;

	config->more_stuff = NULL;

	return config;
}

void
DestroyWharfConfig (WharfConfig * config)
{
    register int i ;

	if (config->bg_color)
		free (config->bg_color);
	if (config->texture_color)
		free (config->texture_color);
	if (config->pixmap)
		free (config->pixmap);
	for( i = 0 ; i< WHEV_MAX_EVENTS ; i++ )
		if (config->sounds[i])
			free (config->sounds[i]);

	while (config->root_folder)
        DestroyWharfButton (&(config->root_folder));

	Destroy_balloonConfig (config->balloon_conf);
	DestroyFreeStorage (&(config->more_stuff));
	free (config);
}

int
print_wharf_folder( WharfButton *folder, int level )
{
    int count = 1 ;
    int my_level = level ;
    while( folder )
    {
        int i = 0;
        show_progress("WHARF.FOLDER[%d].BUTTON[%d].set_flags=0x%lX;", my_level, count, folder->set_flags );
        show_progress("WHARF.FOLDER[%d].BUTTON[%d].title=\"%s\";", my_level, count, folder->title );
        if( folder->icon )
            while( folder->icon[i] != NULL )
            {
                show_progress("WHARF.FOLDER[%d].BUTTON[%d].icon[%d]=\"%s\";", my_level, count, i, folder->icon[i] );
                ++i;
            }
        if( folder->function )
            print_func_data(__FILE__, __FUNCTION__, __LINE__, folder->function);
        else
            show_progress( "no function attached" );
        if( folder->folder )
            level = print_wharf_folder( folder->folder, level+1 );
        ++count ;
        folder = folder->next ;
    }
    return level;
}

void
PrintWharfConfig(WharfConfig *config )
{
    show_progress( "WHARF.flags=0x%lX;", config->flags );
    show_progress( "WHARF.set_flags=0x%lX;", config->set_flags );
    if( get_flags(config->set_flags, WHARF_ROWS) )
        show_progress( "WHARF.rows=%d;", config->rows );

    if( get_flags(config->set_flags, WHARF_COLUMNS) )
        show_progress( "WHARF.columns=%d;", config->columns );

    if( get_flags( config->set_flags, WHARF_GEOMETRY ) )
        show_progress( "WHARF.geometry=(0x%lx,%dx%d%+d%+d);", config->geometry.flags, config->geometry.width, config->geometry.height, config->geometry.x, config->geometry.y);

    if( get_flags( config->set_flags, WHARF_WITHDRAW_STYLE ) )
        show_progress( "WHARF.withdraw_style=%d;", config->withdraw_style );

    if( get_flags( config->set_flags, WHARF_FORCE_SIZE ) )
        show_progress( "WHARF.force_size=(0x%lx,%dx%d%+d%+d);", config->force_size.flags, config->force_size.width, config->force_size.height, config->force_size.x, config->force_size.y);

    if( get_flags( config->set_flags, WHARF_ANIMATE_STEPS ) )
        show_progress( "WHARF.animate_steps=%d;", config->animate_steps );
    if( get_flags( config->set_flags, WHARF_ANIMATE_STEPS_MAIN ) )
        show_progress( "WHARF.animate_steps_main=%d;", config->animate_steps_main );
    if( get_flags( config->set_flags, WHARF_ANIMATE_DELAY ) )
        show_progress( "WHARF.animate_delay=%d;", config->animate_delay );

    if( get_flags( config->set_flags, WHARF_SOUND ) )
    {
        int i ;
        for( i = 0 ; i < WHEV_MAX_EVENTS ; ++i )
            show_progress( "WHARF.sounds[%d]=\"%s\";", i, config->sounds[i] );
    }

    print_wharf_folder( config->root_folder, 1 );

}

void print_trimmed_str( char *prompt, char * str );

unsigned long
WharfSpecialFunc (ConfigDef * config, FreeStorageElem ** storage)
{
	TermDef      *pterm;
	register char *cur;

    LOCAL_DEBUG_CALLER_OUT("%p,%p", config, storage);
	if (config == NULL || storage == NULL)
		return SPECIAL_BREAK;

	/* checking if we have ~Folders in here */
    LOCAL_DEBUG_OUT("checking for ~folders at :%s", "" );
    print_trimmed_str( "config->tdata", config->tdata );
    print_trimmed_str( "config->tline", config->tline );
    print_trimmed_str( "config->cursor", config->cursor );
    if ((pterm = FindStatementTerm (config->tdata, &WharfSyntax)) == NULL)
        if (mystrncasecmp (config->tdata, "~Folders", 7) == 0)
        {
            show_error( " config line %d: ~Folders keyword is no longer supported. \nPlease Update your configuration to use ~Folder instead!\n Please Accept our apologies for any inconvinience.", config->line_count);
            pterm = FindStatementTerm ("~Folder", &WharfSyntax);
        }

    if( pterm != NULL )
	{
        LOCAL_DEBUG_OUT("term %p found keyword :[%s]", pterm, pterm->keyword );
        if (pterm->id == WHARF_FolderEnd_ID)
		{
			config->current_term = pterm;
			/* we are 2 levels deep, and FolderEnd will get us only 1 level up
               so we need to climb another level ourselves : */
            LOCAL_DEBUG_OUT( "folder end - Poping out%s", "");
			PopSyntax (config);
			PopStorage (config);
			return SPECIAL_SKIP;			   /* don't care what will happen */
		}
	}
    /* processing wharf item name and icons : */
	ProcessStatement (config);
    /* since we have have subconfig of Functions that has \n as line terminator
     * we are going to get entire line again at config->cursor
     * so lets skip 3 tokens of <name> <icon>, since those are not parts
     * of following function */
    print_trimmed_str("skiping 2 tokens at", config->tdata );
    cur = tokenskip( config->tdata, 2 );
    print_trimmed_str("skipped to", cur );
	if (*cur != '\0')
	{
		char         *good_cursor;
		TermDef      *pterm;

        good_cursor = config->cursor ;
		config->cursor = cur;
        /* we are at the beginning of the function definition right now - lets process it :*/
        /* read in entire function definition */
		GetNextStatement (config, 1);
        /* lets find us the term for this definition */
        print_trimmed_str( "config->current_data", config->current_data );
        LOCAL_DEBUG_OUT( "curr_data_len = %d", config->current_data_len);
        print_trimmed_str("checking keyword at", config->tline );
		if ((pterm = FindStatementTerm (config->tline, config->syntax)) == NULL)
        {   /* courtesy check for mistyped Folder keyword : */
			if (mystrncasecmp (config->tline, "Folders", 7) == 0)
            {
                show_error( " config line %d: Folders keyword is no longer supported. \nPlease Update your configuration to use Folder instead!\n Please Accept our apologies for any inconvinience.", config->line_count);
                pterm = FindStatementTerm ("Folder", config->syntax);
            }
        }

        if( pterm == NULL )
        {
            /* we are 2 levels deep, and FolderEnd will get us only 1 level up
               so we need to climb another level ourselves : */
			PopSyntax (config);
			PopStorage (config);
        }else
        {   /* we have a valid function definition : */
			config->current_term = pterm;
            /* we do not want to continue processing the rest of the config as
             * a functions : */
			config->current_flags |= CF_LAST_OPTION;
            /* some wierd code to handle the fact that Folder is not really a function,
             * but instead a start for new nested set of Wharf items : */
            LOCAL_DEBUG_OUT("processing function definition statement...%s","");
			ProcessStatement (config);
			if (config->current_term->id == F_Folder)
            {   /* in which case we let parser to carry on the parsing of the Folder item,
                 * which will get us into nested WharfSyntax subsyntax */
                config->current_flags &= ~CF_LAST_OPTION;
			}
		}
        /* restarting parsing from the same location : */
        if( config->cursor < good_cursor )
            config->cursor = good_cursor;
        LOCAL_DEBUG_OUT("done processing function definition statement...%s","");
	}
    print_trimmed_str( "config->tdata", config->tdata );
    print_trimmed_str( "config->tline", config->tline );
    print_trimmed_str( "config->cursor", config->cursor );
    /* already done processing current statement - let parser know about it : */
    return SPECIAL_SKIP;
}

void ParseWharfFolder (FreeStorageElem ** storage_tail, WharfButton ** tail);

void
ParseWharfItem (FreeStorageElem * storage, WharfButton **folder)
{
    WharfButton *wb = *folder, **insert = folder ;
    Bool no_title ;

    if (storage == NULL || folder == NULL)
        return;
    if (storage->argc < 2)
        return;
    no_title = (storage->argv[0][0] == '-' && storage->argv[0][1] == '\0') ||
               (mystrcasecmp( storage->argv[0], "nil") == 0) ;
    insert = folder ;
    if( !no_title )
    {
        while( wb != NULL && ( wb->title == NULL || strcmp( wb->title, storage->argv[0]) != 0 ))
        {
            insert = &(wb->next) ;
            wb = wb->next ;
        }
    }else
        while( wb != NULL )
        {
            insert = &(wb->next) ;
            wb = wb->next ;
        }

    if (wb == NULL)
    {
        if ((wb = CreateWharfButton ()) == NULL)
            return;
        *insert = wb ;
    }
    if (wb->title)
        free (wb->title);
    wb->title = mystrdup (storage->argv[0]);

    {
        char **new_icon_list = comma_string2list (storage->argv[1]);
        if (new_icon_list)
        {
            register char *ptr;
            register int  null_icon = 0;

            if ((ptr = new_icon_list[0]) == NULL)
                null_icon++;
            else if (*(ptr) == '-' && *(ptr + 1) == '\0')
                null_icon++;
            else if (mystrcasecmp (ptr, "nil") == 0)
                null_icon++;

            if (null_icon > 0)
            {
                if (new_icon_list[0] != NULL)
                    free (new_icon_list[0]);
                free (new_icon_list);
                new_icon_list = NULL;
            }
        }

        if( new_icon_list != NULL )
        {
            if (wb->icon)
               free (wb->icon);
            wb->icon = new_icon_list ;
        }
    }

LOCAL_DEBUG_OUT( "wharf button \"%s\" has substorage set to %p", wb->title, storage->sub );
	if (storage->sub)
	{
		FreeStorageElem *pstorage = storage->sub;
		register TermDef *pterm = pstorage->term;

		if (pterm != NULL)
		{
LOCAL_DEBUG_OUT( "term for keyword \"%s\" found in substorage", pterm->keyword );
            if( pterm->id == F_Folder )
            {
				 if (pstorage->sub)
				 {
					 pstorage = pstorage->sub;
                     ParseWharfFolder (&pstorage, &(wb->folder));
				 }
            }else if (pterm->type == TT_FUNCTION)
			{
				ConfigItem    item;
				item.memory = NULL;
				if (ReadConfigItem (&item, pstorage))
				{
                    if (wb->function)
                        destroy_func_data (&(wb->function));
                    wb->function = item.data.function;
				}
			}
		}
	}
}

void
ParseWharfFolder (FreeStorageElem ** storage_tail, WharfButton ** folder)
{
    if (storage_tail != NULL && folder != NULL)
    {
        register FreeStorageElem *folder_storage = (*storage_tail);
        while (folder_storage != NULL)
        {
            if (folder_storage->term->id != WHARF_Wharf_ID)
                break;
            ParseWharfItem (folder_storage, folder);
            /* keep parameter pointing to the last processed item */
            *storage_tail = folder_storage;
            /* while advancing internal pointer ahead.
            we have to do that as our caller will
            expect storage_tail to be pointing to last  processed item */
            folder_storage = folder_storage->next;
        }
    }
}



WharfConfig  *
ParseWharfOptions (const char *filename, char *myname)
{
	ConfigDef    *ConfigReader = InitConfigReader (myname, &WharfSyntax, CDT_Filename, (void *)filename,
												   WharfSpecialFunc);
	WharfConfig  *config = CreateWharfConfig ();
	FreeStorageElem *Storage = NULL, *pCurr;
	ConfigItem    item;
	MyStyleDefinition **styles_tail = &(config->style_defs);

	if (!ConfigReader)
		return config;

    FuncTerms[F_Folder].sub_syntax = &WharfSyntax ;

    item.memory = NULL;
	PrintConfigReader (ConfigReader);
SHOW_CHECKPOINT;
	ParseConfig (ConfigReader, &Storage);
SHOW_CHECKPOINT;
	PrintFreeStorage (Storage);

	/* getting rid of all the crap first */
	StorageCleanUp (&Storage, &(config->more_stuff), CF_DISABLED_OPTION);
SHOW_CHECKPOINT;
	config->balloon_conf = Process_balloonOptions (Storage, NULL);

	for (pCurr = Storage; pCurr; pCurr = pCurr->next)
	{
		if (pCurr->term == NULL)
			continue;
        if (ReadFlagItem (&(config->set_flags), &(config->flags), pCurr, WharfFlags))
		{
            continue;
		}
		if (!ReadConfigItem (&item, pCurr))
			continue;
		switch (pCurr->term->id)
		{
		 case WHARF_Wharf_ID:
			 item.ok_to_free = 1;
             ParseWharfFolder (&pCurr, &(config->root_folder));
			 break;
		 case WHARF_Geometry_ID:
			 set_flags (config->set_flags, WHARF_GEOMETRY);
			 config->geometry = item.data.geometry;
			 break;
		 case WHARF_Rows_ID:
			 set_flags (config->set_flags, WHARF_ROWS);
			 config->rows = item.data.integer;
			 break;
		 case WHARF_Columns_ID:
			 set_flags (config->set_flags, WHARF_COLUMNS);
			 clear_flags (config->set_flags, WHARF_ROWS);
			 config->columns = item.data.integer;
			 break;
		 case WHARF_WithdrawStyle_ID:
			 set_flags (config->set_flags, WHARF_WITHDRAW_STYLE);
			 config->withdraw_style = item.data.integer;
			 break;
		 case WHARF_ForceSize_ID:
			 set_flags (config->set_flags, WHARF_FORCE_SIZE);
			 config->force_size = item.data.geometry;
			 /* errorneous value check */
			 if (!(config->force_size.flags & WidthValue))
				 config->force_size.width = 64;
			 if (!(config->force_size.flags & HeightValue))
				 config->force_size.height = 64;
			 config->force_size.flags = WidthValue | HeightValue;
			 break;
		 case WHARF_TextureType_ID:
			 set_flags (config->set_flags, WHARF_TEXTURE_TYPE);
			 config->texture_type = item.data.integer;
			 break;
		 case WHARF_MaxColors_ID:
			 set_flags (config->set_flags, WHARF_MAX_COLORS);
			 config->max_colors = item.data.integer;
			 break;
		 case WHARF_BgColor_ID:
			 set_string_value (&(config->bg_color), item.data.string, &(config->set_flags), WHARF_BG_COLOR);
			 break;
		 case WHARF_TextureColor_ID:
			 set_string_value (&(config->texture_color), item.data.string, &(config->set_flags), WHARF_TEXTURE_COLOR);
			 break;
		 case WHARF_Pixmap_ID:
			 set_string_value (&(config->pixmap), item.data.string, &(config->set_flags), WHARF_PIXMAP);
			 break;
		 case WHARF_AnimateStepsMain_ID:
			 set_flags (config->set_flags, WHARF_ANIMATE_STEPS_MAIN);
			 config->animate_steps_main = item.data.integer;
			 break;
		 case WHARF_AnimateSteps_ID:
			 set_flags (config->set_flags, WHARF_ANIMATE_STEPS);
			 config->animate_steps = item.data.integer;
			 break;
		 case WHARF_AnimateDelay_ID:
			 set_flags (config->set_flags, WHARF_ANIMATE_DELAY);
			 config->animate_delay = item.data.integer;
			 break;
		 case WHARF_Sound_ID:
			 if (pCurr->sub == NULL)
			 {
				 if (pCurr->argc > 0)
				 {
					 register char *sound = mystrdup (pCurr->argv[pCurr->argc - 1]);

					 set_string_value (&(config->sounds[WHEV_PUSH]), sound, &(config->set_flags), WHARF_SOUND);
				 }
			 } else if (pCurr->sub->argc > 0)
			 {
				 register char *sound = mystrdup (pCurr->sub->argv[0]);

				 set_string_value (&(config->sounds[WHEV_Id2Code (pCurr->sub->term->id)]),
								   sound, &(config->set_flags), WHARF_SOUND);
			 }
			 item.ok_to_free = 1;
			 break;
         case WHARF_LabelLocation_ID :
             set_flags (config->set_flags, WHARF_LABEL_LOCATION);
             config->label_location = item.data.integer;
             break ;
         case WHARF_AlignContents_ID :
             set_flags (config->set_flags, WHARF_ALIGN_CONTENTS);
             config->align_contents = item.data.integer;
             break ;
         case MYSTYLE_START_ID:
			 styles_tail = ProcessMyStyleOptions (pCurr->sub, styles_tail);
			 item.ok_to_free = 1;
			 break;

		 default:
			 if (pCurr->term->type != TT_FLAG)
				 item.ok_to_free = 1;
		}
	}
	ReadConfigItem (&item, NULL);
SHOW_CHECKPOINT;
    DestroyConfig (ConfigReader);
SHOW_CHECKPOINT;
    DestroyFreeStorage (&Storage);
SHOW_CHECKPOINT;
    return config;
}

#if 0

FreeStorageElem **WharfFolder2FreeStorage (SyntaxDef * syntax, FreeStorageElem ** tail, WharfButton * folder, int root);

FreeStorageElem **
WharfButton2FreeStorage (SyntaxDef * syntax, FreeStorageElem ** tail, WharfButton * button)
{
	FreeStorageElem *new_elem = NULL;
	TermDef      *pterm = FindTerm (syntax, TT_ANY, WHARF_Wharf_ID);

	if (pterm == NULL || button == NULL || tail == NULL)
		return tail;
	if (button->title == NULL)
		return tail;

	/* adding WharfButton free storage here */

	if ((new_elem = AddFreeStorageElem (syntax, tail, pterm, WHARF_Wharf_ID)) != NULL)
	{
		char         *icon = list2comma_string (button->icon);
		int           title_len, len;

		len = title_len = strlen (button->title) + 1;
		len += ((icon) ? strlen (icon) : 1) + 1;
		if (len > 0)
		{
			new_elem->argc = 2;
			new_elem->argv = CreateStringArray (2);
			new_elem->argv[0] = safemalloc (len);
			new_elem->argv[1] = &(new_elem->argv[0][title_len + 1]);
			strcpy (new_elem->argv[0], button->title);
			if (icon == NULL)
				strcpy (new_elem->argv[1], "-");
			else
			{
				strcpy (new_elem->argv[1], icon);
				free (icon);
			}
			tail = &(new_elem->next);

			if (button->folder)
			{
				FunctionData  tmp;

				memset (&tmp, 0x00, sizeof (FunctionData));
				tmp.func = F_Folder;
				Func2FreeStorage (&FuncSyntax, &(new_elem->sub), &tmp);
				WharfFolder2FreeStorage (syntax, &(new_elem->sub->sub), button->folder, False);

			} else if (button->function)
			{
				Func2FreeStorage (&FuncSyntax, &(new_elem->sub), button->function);
			}

		}
	}
	return tail;
}

FreeStorageElem **
WharfFolder2FreeStorage (SyntaxDef * syntax, FreeStorageElem ** tail, WharfButton * folder, int root)
{
	if (folder == NULL || tail == NULL)
		return tail;

	/* adding balloon free storage here */
	while (folder)
	{
		tail = WharfButton2FreeStorage (syntax, tail, folder);
		folder = folder->next;
	}
	if (!root)
	{
		char         *fend = WHARF_FOLDER_END;

		tail = String2FreeStorage (syntax, tail, fend, WHARF_Wharf_ID);
	}

	return tail;
}

/* returns:
 *            0 on success
 *              1 if data is empty
 *              2 if ConfigWriter cannot be initialized
 *
 */
int
WriteWharfOptions (const char *filename, char *myname, WharfConfig * config, unsigned long flags)
{
	ConfigDef    *WharfConfigWriter = NULL;
	FreeStorageElem *Storage = NULL, **tail = &Storage;
	int i ;

	if (config == NULL)
		return 1;

    FuncTerms[F_Folder].sub_syntax = &WharfSyntax ;

    if ((WharfConfigWriter = InitConfigWriter (myname, &WharfSyntax, CDT_Filename, (void *)filename)) == NULL)
		return 2;

    CopyFreeStorage (&Storage, config->more_stuff);

	if (config->balloon_conf)
		tail = balloon2FreeStorage (&WharfSyntax, tail, config->balloon_conf);
    if (config->style_defs)
        tail = MyStyleDefs2FreeStorage (&WharfSyntax, tail, config->style_defs);
	/* building free storage here */
	/* geometry */
	if (get_flags (config->set_flags, WHARF_GEOMETRY))
		tail = Geometry2FreeStorage (&WharfSyntax, tail, &(config->geometry), WHARF_Geometry_ID);
	if (get_flags (config->set_flags, WHARF_FORCE_SIZE))
		tail = Geometry2FreeStorage (&WharfSyntax, tail, &(config->force_size), WHARF_ForceSize_ID);
	/* Integer values : */
	/* rows */
	if (get_flags (config->set_flags, WHARF_ROWS))
        tail = Integer2FreeStorage (&WharfSyntax, tail, NULL, config->rows, WHARF_Rows_ID);
	/* columns */
	if (get_flags (config->set_flags, WHARF_COLUMNS))
        tail = Integer2FreeStorage (&WharfSyntax, tail, NULL, config->columns, WHARF_Columns_ID);
	/* withdraw_style */
	if (get_flags (config->set_flags, WHARF_WITHDRAW_STYLE))
        tail = Integer2FreeStorage (&WharfSyntax, tail, NULL, config->withdraw_style, WHARF_WithdrawStyle_ID);
	/* texture_type  */
	if (get_flags (config->set_flags, WHARF_TEXTURE_TYPE))
        tail = Integer2FreeStorage (&WharfSyntax, tail, NULL, config->texture_type, WHARF_TextureType_ID);
	/* max_colors */
	if (get_flags (config->set_flags, WHARF_MAX_COLORS))
        tail = Integer2FreeStorage (&WharfSyntax, tail, NULL, config->max_colors, WHARF_MaxColors_ID);
	/* animate_steps */
	if (get_flags (config->set_flags, WHARF_ANIMATE_STEPS))
        tail = Integer2FreeStorage (&WharfSyntax, tail, NULL, config->animate_steps, WHARF_AnimateSteps_ID);
	/* animate_steps_main */
	if (get_flags (config->set_flags, WHARF_ANIMATE_STEPS_MAIN))
        tail = Integer2FreeStorage (&WharfSyntax, tail, NULL, config->animate_steps_main, WHARF_AnimateStepsMain_ID);
	/* animate_delay */
	if (get_flags (config->set_flags, WHARF_ANIMATE_DELAY))
        tail = Integer2FreeStorage (&WharfSyntax, tail, NULL, config->animate_delay, WHARF_AnimateDelay_ID);


	/* Flags : */
	tail = Flags2FreeStorage (&WharfSyntax, tail, WharfFlags, 0xFFFFFFFF, config->set_flags);

	/* StringValues */
	/* bg_color */
	if (get_flags (config->set_flags, WHARF_BG_COLOR))
		tail = String2FreeStorage (&WharfSyntax, tail, config->bg_color, WHARF_BgColor_ID);

	/* texture_color */
	if (get_flags (config->set_flags, WHARF_TEXTURE_COLOR))
		tail = String2FreeStorage (&WharfSyntax, tail, config->texture_color, WHARF_TextureColor_ID);

	/* pixmap name */
	if (get_flags (config->set_flags, WHARF_PIXMAP))
		tail = String2FreeStorage (&WharfSyntax, tail, config->pixmap, WHARF_Pixmap_ID);

	/* sound */
	if (get_flags (config->set_flags, WHARF_SOUND))
		for( i = 0 ; i < WHEV_MAX_EVENTS ; i++ )
			tail = String2FreeStorage (&WharfSyntax, tail, config->sounds[i], WHARF_Sound_ID);

	/* Writing Wharf Entries : */
	if (config->root_folder)
		tail = WharfFolder2FreeStorage (&WharfSyntax, tail, config->root_folder, True);

	/* writing config into the file */
	WriteConfig (WharfConfigWriter, &Storage, CDT_Filename, (void **)&filename, flags);
	DestroyConfig (WharfConfigWriter);

	if (Storage)
	{
		fprintf (stderr, "\n%s:Config Writing warning: Not all Free Storage discarded! Trying again...", myname);
		DestroyFreeStorage (&Storage);
		fprintf (stderr, (Storage != NULL) ? " failed." : " success.");
	}
	return 0;
}

#endif