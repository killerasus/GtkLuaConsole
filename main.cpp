/* 
 * File:   main.cpp
 * Author: baraujo
 *
 * Created on May 13, 2014, 2:36 PM
 */

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <gtk/gtk.h>
#include <lua.hpp>
#include <gtk-2.0/gtk/gtkdialog.h>
#include <string>

/**
 * Prints to the GtkTextView defined in the closure
 * @param lua
 * @return 
 */
static int consolePrint( lua_State* lua )
{
    void* pointer = lua_touserdata( lua, lua_upvalueindex(1) );
    assert( pointer != NULL );
    GtkTextBuffer* buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( pointer ) );
    
    int nargs = lua_gettop( lua );
    
    for (int i=1; i <= nargs; ++i) {
		const char* string = lua_tostring( lua, i );
        gtk_text_buffer_insert_at_cursor( buffer, string, -1 );
        
        if (i < nargs)
            gtk_text_buffer_insert_at_cursor( buffer, "\t", -1 );
    }
    
    gtk_text_buffer_insert_at_cursor( buffer, "\n", -1 );
    
    return 0;
}

/**
 * Destroy event
 * @param object
 * @param data
 */
void onWindowDestroy( GtkWidget* object, gpointer data )
{
    gtk_main_quit();
}

/**
 * Cancell button callback
 * @param object
 * @param data
 */
void cancelCallback( GtkWidget* object, gpointer data )
{
    gtk_main_quit();
}

/**
 * Cleans the text entry buffer
 * @param entry
 */
void clearEntry( GtkEntry* entry )
{
    gtk_entry_set_text( entry, "" );
}

/**
 * Process the command string in the current Lua state
 * @param commandString
 * @param buffer
 * @param lua
 */
void processString( const char* commandString, GtkTextBuffer* buffer, lua_State* lua )
{
    gtk_text_buffer_insert_at_cursor( buffer, commandString, -1 );
    gtk_text_buffer_insert_at_cursor( buffer, "\n", -1 );
    
    int error;
    
    error = luaL_loadbuffer( lua, commandString, strlen(commandString), "line" ) ||
            lua_pcall( lua, 0, 0, 0 );
    
    if (error)
    {
        gtk_text_buffer_insert_at_cursor( buffer, lua_tostring( lua, -1 ), -1 );
        gtk_text_buffer_insert_at_cursor( buffer, "\n", -1 );
        lua_pop( lua, -1 );
    }
}

/**
 * Process the script file in the current Lua state
 * @param fileName
 * @param buffer
 * @param lua
 */
void processFile( const char* fileName, GtkTextBuffer* buffer, lua_State* lua )
{
    if (strlen(fileName) > 0)
    {
        std::string message = "Loading file: ";
        message.append( fileName );

        gtk_text_buffer_insert_at_cursor( buffer, message.c_str(), -1 );
        gtk_text_buffer_insert_at_cursor( buffer, "\n", -1 );

        int error;
        
        error = luaL_loadfile( lua, fileName ) || lua_pcall( lua, 0, 0, 0 );
        
        if (error)
        {
            gtk_text_buffer_insert_at_cursor( buffer, lua_tostring( lua, -1 ), -1 );
            gtk_text_buffer_insert_at_cursor( buffer, "\n", -1 );
            lua_pop( lua, -1 );
        }
    }
    else
    {
        gtk_text_buffer_insert_at_cursor( buffer, "File name empty.", -1 );
        gtk_text_buffer_insert_at_cursor( buffer, "\n", -1 );
    }
}

/**
 * Process the enter event from the GtkTextEntry
 * @param object
 * @param data      Pointer to textView
 */
void processStringCallback( GtkWidget* object, gpointer data )
{
    void** tuple = static_cast<void**>(data);
    
    const gchar* text = gtk_entry_get_text ( GTK_ENTRY(object) );
    GtkTextBuffer* buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(tuple[0]) );
    lua_State* state = static_cast<lua_State*>( tuple[1] );
    
    processString( text, buffer, state );
    
    clearEntry( GTK_ENTRY(object) );
}


/**
 * Process the click event on execute button
 * @param object
 * @param data      Um vetor de void* com 2 posicoes {textView, textEntry}
 */
void processStringButtonCallback( GtkWidget* object, gpointer data )
{
    void** tuple = static_cast<void**>(data);
    
    GtkTextBuffer* buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(tuple[0]) );
    const gchar* text = gtk_entry_get_text ( GTK_ENTRY(tuple[1]) );
    lua_State* state = static_cast<lua_State*>( tuple[2] );
    
    processString( text, buffer, state );
    
    clearEntry( GTK_ENTRY(tuple[1]) );
}

/**
 * 
 * @param object
 * @param data
 */
void loadFileButtonCallback( GtkWidget* object, gpointer data )
{
    void** tuple = static_cast<void**>(data);
    GtkTextView* view = GTK_TEXT_VIEW( tuple[0] );
    GtkDialog* dialog = GTK_DIALOG ( tuple[1] );
    lua_State* lua = static_cast<lua_State*>( tuple[2] );
    GtkTextBuffer* buffer = gtk_text_view_get_buffer( view );
    gint res = gtk_dialog_run ( dialog );
    
    if (res == GTK_RESPONSE_OK)
    {
        const char* fileName;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        
        fileName = gtk_file_chooser_get_filename( chooser );
        
#ifdef _DEBUG
        printf( "Selected file: %s\n", fileName );
#endif    
        processFile( fileName, buffer, lua );
    }
    
    gtk_widget_hide( GTK_WIDGET(dialog) );
}

/*
 * Lua console example in Gtk
 */
int main( int argc, char** argv ) 
{
    GtkBuilder* builder;
    GtkWidget* window;
    GtkWidget* cancelButton;
    GtkWidget* textEntry;
    GtkWidget* executeButton;
    GtkWidget* textView;
    GtkWidget* fileChooserDialog;
    GtkWidget* loadFileButton;
    GtkFileFilter* fileFilter;
    lua_State* luaState;
    void* tupleViewState[2];
    void* tupleViewEntryState[3];
    void* tupleViewDialogState[3];
    
    luaState = luaL_newstate();
    luaL_openlibs( luaState );
    
    gtk_init( &argc, &argv );
    
    builder = gtk_builder_new();
    gtk_builder_add_from_file( builder, "glade/console.glade", NULL );

    window = GTK_WIDGET( gtk_builder_get_object( builder, "window1" ) );
    g_signal_connect( window, "destroy", G_CALLBACK(onWindowDestroy), NULL );
    gtk_window_set_title( GTK_WINDOW(window), "Lua Console" );
    
    textView = GTK_WIDGET( gtk_builder_get_object( builder, "textview1" ) );
    
    // Creates a closure for ConsolePrint with GtkTextBuffer
    // Closures allow the method to access data without having to pass it as a parameter in each call
    
    // Pushing items to the stack
    lua_pushlightuserdata( luaState, textView );
    // Creates a closure with the (1) items in the stack. Each item is accessible indexed by its stack position
    // see consolePrint implementation for an example
    lua_pushcclosure( luaState, consolePrint, 1 );
    // Registers the function in the stack in the global table with the key
    lua_setglobal( luaState, "ConsolePrint" );
    
    cancelButton = GTK_WIDGET( gtk_builder_get_object( builder, "button2" ) );
    g_signal_connect( cancelButton, "clicked", G_CALLBACK(cancelCallback), NULL );
    
    tupleViewState[0] = textView;
    tupleViewState[1] = luaState;
    
    textEntry = GTK_WIDGET( gtk_builder_get_object( builder, "entry1" ) );
    g_signal_connect( textEntry, "activate", G_CALLBACK(processStringCallback), tupleViewState );
    
    tupleViewEntryState[0] = textView;
    tupleViewEntryState[1] = textEntry;
    tupleViewEntryState[2] = luaState;
    
    executeButton = GTK_WIDGET( gtk_builder_get_object( builder, "button1" ) );
    g_signal_connect( executeButton, "clicked", G_CALLBACK(processStringButtonCallback), tupleViewEntryState );
    
    fileChooserDialog = GTK_WIDGET( gtk_builder_get_object( builder, "filechooserdialog1" ) );
    gtk_dialog_add_button( GTK_DIALOG(fileChooserDialog), "Cancel", GTK_RESPONSE_CANCEL );
    gtk_dialog_add_button( GTK_DIALOG(fileChooserDialog), "OK", GTK_RESPONSE_OK );
    
    fileFilter = GTK_FILE_FILTER( gtk_builder_get_object( builder, "filefilter1" ) );
    gtk_file_filter_set_name( fileFilter, "Lua scripts" );
    gtk_file_filter_add_pattern( fileFilter, "*.lua" );
    
    tupleViewDialogState[0] = textView;
    tupleViewDialogState[1] = fileChooserDialog;
    tupleViewDialogState[2] = luaState;
    
    loadFileButton = GTK_WIDGET( gtk_builder_get_object( builder, "button3" ) );
    g_signal_connect( loadFileButton, "clicked", G_CALLBACK(loadFileButtonCallback), tupleViewDialogState );

    gtk_widget_show_all( window );
    gtk_main();
    
    lua_close( luaState );
    g_object_unref( G_OBJECT(builder) );

    return 0;
}

