#include <wx/wx.h>
#include <wx/statline.h>
#include <wx/html/htmlwin.h>
#include <wx/tokenzr.h>
#include <wx/stc/stc.h>
#include <wx/splitter.h>

#include <wex/codeedit.h>

class OutputWindow;
static OutputWindow *__g_outputWindow = 0;


class OutputWindow : public wxFrame
{
private:
	wxTextCtrl *m_text;
public:
	OutputWindow() : wxFrame( 0, wxID_ANY, "Output Window", wxDefaultPosition, wxSize(900, 200) )
	{
#ifdef __WXMSW__
		SetIcon( wxICON( appicon ) );
#endif
		m_text = new wxTextCtrl( this,  wxID_ANY, "Ready\n", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	}
	virtual ~OutputWindow() { __g_outputWindow = 0; }
	void CloseEventHandler( wxCloseEvent & ) { Hide(); }
	void Append( const wxString &text ) { m_text->AppendText( text ); }
	void Clear() { m_text->Clear(); }
	DECLARE_EVENT_TABLE()
};
BEGIN_EVENT_TABLE(OutputWindow, wxFrame)
	EVT_CLOSE( OutputWindow::CloseEventHandler )
END_EVENT_TABLE()

void ShowOutputWindow()
{
	if (__g_outputWindow == 0)
		__g_outputWindow = new OutputWindow();

	if ( !__g_outputWindow->IsShown())
	{
		__g_outputWindow->Show();
		__g_outputWindow->Raise();
	}
}

void HideOutputWindow()
{
	if (__g_outputWindow) __g_outputWindow->Hide();
}

void Output( const wxString &text )
{
	if (__g_outputWindow) __g_outputWindow->Append( text );
}


void Output( const char *fmt, ... )
{
	static char buf[2048];
	va_list ap;
	va_start(ap, fmt);
#if defined(_MSC_VER)||defined(_WIN32)
	_vsnprintf(buf, 2046, fmt, ap);
#else
	vsnprintf(buf, 2046, fmt, ap);
#endif
	va_end(ap);
	Output( wxString(buf) );	
}

void ClearOutput()
{
	if (__g_outputWindow) __g_outputWindow->Clear();
}

void DestroyOutputWindow()
{
	if (__g_outputWindow) __g_outputWindow->Destroy();
}



enum { ID_CODEEDITOR = wxID_HIGHEST+1, ID_RUN, ID_COMPILE, ID_OUTPUT_WINDOW,
	ID_FIND, ID_FIND_NEXT, ID_REPLACE, ID_REPLACE_NEXT };

static int __ndoc = 0;


class EditorWindow : public wxFrame
{
private:
	static int __s_numEditorWindows;
	
	wxCodeEditCtrl *m_editor;
	wxString m_fileName;
public:
	EditorWindow()
		: wxFrame( 0, wxID_ANY, wxString::Format("untitled %d",++__ndoc), wxDefaultPosition, wxSize(700,700) )
	{
		__s_numEditorWindows++;

#ifdef __WXMSW__
		SetIcon( wxICON( appicon ) );
#endif
		
		SetBackgroundColour( *wxWHITE );
		
		wxMenu *file_menu = new wxMenu;
		file_menu->Append( wxID_NEW, "New\tCtrl-N" );
		file_menu->AppendSeparator();
		file_menu->Append( wxID_OPEN, "Open\tCtrl-O");
		file_menu->Append( wxID_SAVE, "Save\tCtrl-S" );
		file_menu->Append( wxID_SAVEAS, "Save as...");
		file_menu->AppendSeparator();
		file_menu->Append( wxID_CLOSE, "Close\tCtrl-W");
		file_menu->AppendSeparator();
		file_menu->Append( wxID_EXIT, "Exit" );

		wxMenu *edit_menu = new wxMenu;
		edit_menu->Append( wxID_UNDO, "Undo\tCtrl-Z" );
		edit_menu->Append( wxID_REDO, "Redo\tCtrl-Y" );
		edit_menu->AppendSeparator();
		edit_menu->Append( wxID_CUT, "Cut\tCtrl-X" );
		edit_menu->Append( wxID_COPY, "Copy\tCtrl-C" );
		edit_menu->Append( wxID_PASTE, "Paste\tCtrl-V" );
		edit_menu->AppendSeparator();
		edit_menu->Append( wxID_SELECTALL, "Select all\tCtrl-A" );
		edit_menu->AppendSeparator();
		edit_menu->Append( ID_FIND, "Find...\tCtrl-F" );
		edit_menu->Append( ID_FIND_NEXT, "Find next\tCtrl-G" );
		edit_menu->Append( ID_REPLACE, "Replace...\tCtrl-H" );
		edit_menu->Append( ID_REPLACE_NEXT, "Replace next\tCtrl-J" );

		wxMenu *act_menu = new wxMenu;
		act_menu->Append( ID_RUN, "Run\tF5" );
		act_menu->Append( ID_COMPILE, "Compile\tF7" );
		act_menu->AppendSeparator();
		act_menu->Append( ID_OUTPUT_WINDOW, "Show output window\tF6" );
		
		wxMenu *help_menu = new wxMenu;
		help_menu->Append( wxID_ABOUT, "About" );
		help_menu->Append( wxID_HELP, "Script reference\tF1");

		wxMenuBar *menu_bar = new wxMenuBar;
		menu_bar->Append( file_menu, "File" );
		menu_bar->Append( edit_menu, "Edit" );
		menu_bar->Append( act_menu, "Actions" );
		menu_bar->Append( help_menu, "Help" );
		
		SetMenuBar( menu_bar );
		
		m_editor = new wxCodeEditCtrl( this, ID_CODEEDITOR );
		m_editor->SetLanguage( wxCodeEditCtrl::CPP );
		m_editor->SetFocus();
	}

	wxString GetFileName() { return m_fileName; }

	void OnCommand( wxCommandEvent &evt )
	{
		switch( evt.GetId() )
		{
		case wxID_NEW:
			(new EditorWindow())->Show();
			break;
		case wxID_OPEN:
			{
				wxFileDialog dlg(this, "Open", wxEmptyString, wxEmptyString,
									"C/C++ Files (*.cpp;*.cxx;*.c;*.h)|*.cpp;*.cxx;*.c;*.h"
									"|Text Files (*.txt);*.txt"
									"|LK Script Files (*.lk);*.lk"
									"|HTML and PHP Files (*.html;*.htm;*.php)|*.html;*.htm;*.php"
									"|VBA Script Files (*.vb;*.vba)|*.vb;*.vba"
									"|Python Script Files (*.py)|*.py"
									"|All Files (*.*)|*.*",
									  wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);

				if (dlg.ShowModal() == wxID_OK)
				{
					wxWindowList wl = ::wxTopLevelWindows;
					for (size_t i=0;i<wl.size();i++)
					{
						if (EditorWindow *ew = dynamic_cast<EditorWindow*>( wl[i] ))
						{
							if ( dlg.GetPath() == ew->GetFileName() )
							{
								ew->Raise();
								return;
							}
						}
					}

					EditorWindow *target = this;
					if (m_editor->GetModify() || !m_fileName.IsEmpty())
					{
						target = new EditorWindow;
						target->Show();
					}
					
					if (!target->Load( dlg.GetPath() ))
						wxMessageBox("Could not load file:\n\n" + dlg.GetPath());
				}
			}
			break;
		case wxID_SAVE:
			Save();
			break;
		case wxID_SAVEAS:
			SaveAs();
			break;
		case wxID_CLOSE:
			Close();
			break;
		case wxID_UNDO: m_editor->Undo(); break;
		case wxID_REDO: m_editor->Redo(); break;
		case wxID_CUT: m_editor->Cut(); break;
		case wxID_COPY: m_editor->Copy(); break;
		case wxID_PASTE: m_editor->Paste(); break;
		case wxID_SELECTALL: m_editor->SelectAll(); break;
		case ID_FIND: m_editor->ShowFindDialog(); break;
		case ID_FIND_NEXT: m_editor->FindNext(); break;
		case ID_REPLACE: m_editor->ShowReplaceDialog(); break;
		case ID_REPLACE_NEXT: m_editor->ReplaceNext(); break;
		case wxID_ABOUT:
			wxMessageBox("Cdev - simple editor tool for Mac OS X");
			break;
		case wxID_HELP:
			{
				wxFrame *frm = new wxFrame( this, wxID_ANY, "Cdev Help", wxDefaultPosition, wxSize(800, 700) );
				frm->SetIcon(wxIcon("appicon"));
				wxHtmlWindow *html = new wxHtmlWindow( frm, wxID_ANY, wxDefaultPosition, wxDefaultSize );

				wxString data( "<html><body><h2>Cdev Help</h2><hr><p>No help currently.</p></body></html>");
				html->SetPage( data );
				frm->Show();
			}
			break;
		case wxID_EXIT:
			{
				wxWindowList wl = ::wxTopLevelWindows;
				for (size_t i=0;i<wl.size();i++)
					if ( dynamic_cast<EditorWindow*>( wl[i] ) != 0 
						&& wl[i] != this )
						wl[i]->Close();

				Close();
			}
			break;
		case ID_RUN:
			wxMessageBox("No run command implemented yet");
			break;
		case ID_COMPILE:
			wxMessageBox("No compile command implemented yet");
			break;
		case ID_OUTPUT_WINDOW:
			ShowOutputWindow();
			break;
		}
	}
	
	bool Save()
	{
		if ( m_fileName.IsEmpty() )
			return SaveAs();
		else
			return Write( m_fileName );
	}

	bool SaveAs()
	{
		wxFileDialog dlg( this, "Save as...",
			wxPathOnly(m_fileName),
			wxFileNameFromPath(m_fileName),
			"LK Script Files (*.lk)|*.lk", wxFD_SAVE|wxFD_OVERWRITE_PROMPT );
		if (dlg.ShowModal() == wxID_OK)
			return Write( dlg.GetPath() );
		else
			return false;
	}

	
	bool CloseDoc()
	{
		if (m_editor->GetModify())
		{
			Raise();
			wxString id = m_fileName.IsEmpty() ? this->GetTitle() : m_fileName;
			int result = wxMessageBox("Document modified. Save it?\n\n" + id, "Query", wxYES_NO|wxCANCEL);
			if ( result == wxCANCEL 
				|| (result == wxYES && !Save()) )
				return false;
		}

		m_editor->SetText(wxEmptyString);
		m_editor->EmptyUndoBuffer();
		m_editor->SetSavePoint();
		m_fileName.Clear();
		SetTitle( wxString::Format("untitled %d",++__ndoc) );
		return true;
	}

	bool Write( const wxString &file )
	{
		if ( m_editor->SaveFile( file ) )
		{
			m_fileName = file;
			wxCodeEditCtrl::Language lang = wxCodeEditCtrl::GetLanguage( file );
			if ( lang != m_editor->GetLanguage() )
				m_editor->SetLanguage( lang );
			SetTitle( wxFileNameFromPath(m_fileName) );
			return true;
		}
		else return false;
	}

	bool Load( const wxString &file )
	{
		FILE *fp = fopen(file.c_str(), "r");
		if ( fp )
		{
			wxString str;
			char buf[1024];
			while(fgets( buf, 1023, fp ) != 0)
				str += buf;

			fclose(fp);
			m_editor->SetText(str);
			m_editor->SetLanguage( wxCodeEditCtrl::GetLanguage( file ) );
			m_editor->EmptyUndoBuffer();
			m_editor->SetSavePoint();
			m_fileName = file;
			SetTitle( wxFileNameFromPath(m_fileName) );
			return true;
		}
		else return false;
	}
	

	void CloseEventHandler( wxCloseEvent &evt )
	{	
		if ( !CloseDoc() )
		{
			evt.Veto();
			return;
		}

		Destroy();
		if (--__s_numEditorWindows == 0)
		{
			DestroyOutputWindow();
			/*
			wxWindowList wl = ::wxTopLevelWindows;
			for (size_t i=0;i<wl.size();i++)
				if (dynamic_cast<PlotWin*>( wl[i] ) != 0)
					wl[i]->Close();
					*/
		}
	}

	void OnEditorModified( wxStyledTextEvent &evt )
	{
		if ( evt.GetModificationType() & wxSTC_MOD_INSERTTEXT 
			|| evt.GetModificationType() & wxSTC_MOD_DELETETEXT )
		{
	//		m_timer.Stop();
	//		m_timer.Start( 500, true );
			evt.Skip();
		}
	}



	DECLARE_EVENT_TABLE()
};
int EditorWindow::__s_numEditorWindows = 0;

BEGIN_EVENT_TABLE( EditorWindow, wxFrame )
	EVT_MENU( wxID_NEW, EditorWindow::OnCommand )
	EVT_MENU( wxID_OPEN, EditorWindow::OnCommand )
	EVT_MENU( wxID_SAVE, EditorWindow::OnCommand )
	EVT_MENU( wxID_SAVEAS, EditorWindow::OnCommand )
	EVT_MENU( wxID_HELP, EditorWindow::OnCommand )
	EVT_MENU( wxID_ABOUT, EditorWindow::OnCommand )
	EVT_MENU( wxID_CLOSE, EditorWindow::OnCommand )
	EVT_MENU( wxID_EXIT, EditorWindow::OnCommand )

	EVT_MENU( wxID_UNDO, EditorWindow::OnCommand )
	EVT_MENU( wxID_REDO, EditorWindow::OnCommand )
	EVT_MENU( wxID_CUT, EditorWindow::OnCommand )
	EVT_MENU( wxID_COPY, EditorWindow::OnCommand )
	EVT_MENU( wxID_PASTE, EditorWindow::OnCommand )
	EVT_MENU( wxID_SELECTALL, EditorWindow::OnCommand )
	EVT_MENU( ID_FIND, EditorWindow::OnCommand )
	EVT_MENU( ID_FIND_NEXT, EditorWindow::OnCommand )
	EVT_MENU( ID_REPLACE, EditorWindow::OnCommand )
	EVT_MENU( ID_REPLACE_NEXT, EditorWindow::OnCommand )

	
	EVT_MENU( ID_RUN, EditorWindow::OnCommand )
	EVT_MENU( ID_COMPILE, EditorWindow::OnCommand )
	EVT_MENU( ID_OUTPUT_WINDOW, EditorWindow::OnCommand )

	EVT_CLOSE( EditorWindow::CloseEventHandler )
END_EVENT_TABLE()


class CdevApp : public wxApp
{
public:
	virtual bool OnInit()
	{
		::wxInitAllImageHandlers();
		(new EditorWindow())->Show();
		return true;
	}
};

IMPLEMENT_APP( CdevApp );

