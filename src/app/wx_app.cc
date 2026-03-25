#include <wx/wx.h>
#include <wx/timer.h>
#include <wx/sound.h>
#include <wx/filedlg.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/dcclient.h>
#include <wx/aboutdlg.h>

#include "slider/board.h"
#include "slider/solver.h"
#include "slider/scrambler.h"

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>

using namespace slider;

struct Theme {
  wxString name;
  wxColour tile_color;
  wxColour tile_highlight;
  wxColour tile_shadow;
  wxColour text_color;
  wxColour bg_primary;
  wxColour bg_secondary;
};

const std::vector<Theme> kThemes = {
    {"Ocean", wxColour(0, 102, 204), wxColour(51, 153, 255), wxColour(0, 51, 153), *wxWHITE, wxColour(240, 248, 255), wxColour(176, 196, 222)},
    {"Forest", wxColour(34, 139, 34), wxColour(50, 205, 50), wxColour(0, 100, 0), *wxWHITE, wxColour(245, 255, 250), wxColour(143, 188, 143)},
    {"Lava", wxColour(255, 69, 0), wxColour(255, 140, 0), wxColour(139, 0, 0), *wxWHITE, wxColour(255, 245, 238), wxColour(255, 218, 185)},
    {"Modern", wxColour(60, 60, 60), wxColour(100, 100, 100), wxColour(30, 30, 30), *wxWHITE, wxColour(240, 240, 240), wxColour(200, 200, 200)},
    {"Cyber", wxColour(128, 0, 128), wxColour(255, 0, 255), wxColour(75, 0, 130), *wxCYAN, wxColour(10, 10, 10), wxColour(40, 40, 40)}
};

class BoardPanel : public wxPanel {
 public:
  BoardPanel(wxWindow* parent, Board* board)
      : wxPanel(parent), board_(board), theme_index_(0) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &BoardPanel::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &BoardPanel::OnMouseDown, this);
    Bind(wxEVT_TIMER, &BoardPanel::OnTimer, this);
    
    animation_timer_.SetOwner(this);
  }

  void SetBoard(Board* board) {
    board_ = board;
    Refresh();
  }

  void SetTheme(int index) {
    if (index >= 0 && index < static_cast<int>(kThemes.size())) {
      theme_index_ = index;
      Refresh();
    }
  }

  void StartAnimation(Direction dir, int tile_val, double duration_s, wxCommandEvent* completion_event = nullptr) {
    anim_dir_ = dir;
    anim_tile_ = tile_val;
    anim_progress_ = 0.0;
    anim_step_ = 0.016 / duration_s; // ~60fps
    animating_ = true;
    completion_event_ = completion_event;
    animation_timer_.Start(16);
  }

  bool IsAnimating() const { return animating_; }

 private:
  void OnPaint(wxPaintEvent& /*event*/) {
    wxAutoBufferedPaintDC dc(this);
    const Theme& theme = kThemes[theme_index_];
    
    wxRect client_rect = GetClientRect();
    dc.GradientFillLinear(client_rect, theme.bg_primary, theme.bg_secondary, wxSOUTH);

    if (!board_) return;

    int size = board_->GetSize();
    if (size <= 0) return;

    int cell_w = client_rect.width / size;
    int cell_h = client_rect.height / size;

    const auto& tiles = board_->GetState().GetTiles();

    for (int i = 0; i < static_cast<int>(tiles.size()); ++i) {
      if (tiles[i] == 0) continue;

      int row = i / size;
      int col = i % size;

      double off_x = 0, off_y = 0;
      if (animating_ && tiles[i] == anim_tile_) {
        // Board::Move(dir) moves the *empty* cell in dir.
        // The tile being animated moves in the OPPOSITE direction into the empty space.
        switch (anim_dir_) {
          case Direction::kUp:    off_y = +anim_progress_ * cell_h; break; // tile slides DOWN into space
          case Direction::kDown:  off_y = -anim_progress_ * cell_h; break; // tile slides UP into space
          case Direction::kLeft:  off_x = +anim_progress_ * cell_w; break; // tile slides RIGHT into space
          case Direction::kRight: off_x = -anim_progress_ * cell_w; break; // tile slides LEFT into space
        }
      }

      wxRect rect(col * cell_w + static_cast<int>(off_x), row * cell_h + static_cast<int>(off_y), cell_w, cell_h);
      rect.Deflate(5);

      dc.SetBrush(wxBrush(wxColour(0, 0, 0, 50)));
      dc.SetPen(*wxTRANSPARENT_PEN);
      dc.DrawRoundedRectangle(rect.x + 3, rect.y + 3, rect.width, rect.height, 10);

      dc.GradientFillLinear(rect, theme.tile_highlight, theme.tile_shadow, wxSOUTH);
      
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.SetPen(wxPen(theme.tile_highlight, 2));
      dc.DrawLine(rect.GetTopLeft(), rect.GetTopRight());
      dc.DrawLine(rect.GetTopLeft(), rect.GetBottomLeft());
      
      dc.SetPen(wxPen(theme.tile_shadow, 2));
      dc.DrawLine(rect.GetBottomLeft(), rect.GetBottomRight());
      dc.DrawLine(rect.GetTopRight(), rect.GetBottomRight());

      dc.SetTextForeground(theme.text_color);
      if (cell_h > 0) {
        wxFont font(cell_h / 3, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
        dc.SetFont(font);
        wxString label = wxString::Format("%d", tiles[i]);
        wxSize text_size = dc.GetTextExtent(label);
        dc.DrawText(label, rect.x + (rect.width - text_size.x) / 2, rect.y + (rect.height - text_size.y) / 2);
      }
    }
  }

  void OnMouseDown(wxMouseEvent& event) {
    if (animating_ || !board_) return;

    int size = board_->GetSize();
    if (size <= 0) return;
    wxSize client_size = GetClientSize();
    int cell_w = client_size.x / size;
    int cell_h = client_size.y / size;
    if (cell_w <= 0 || cell_h <= 0) return;

    int col = event.GetX() / cell_w;
    int row = event.GetY() / cell_h;

    if (row >= 0 && row < size && col >= 0 && col < size) {
      int pos = row * size + col;
      const auto& tiles = board_->GetState().GetTiles();
      if (pos >= 0 && pos < static_cast<int>(tiles.size())) {
        int val = tiles[pos];
        if (val != 0) {
          auto dir = board_->GetDirectionToMoveTile(val);
          if (dir) {
            wxCommandEvent evt(wxEVT_BUTTON, GetId());
            evt.SetInt(val);
            ProcessWindowEvent(evt);
          }
        }
      }
    }
  }

  void OnTimer(wxTimerEvent& /*event*/) {
    anim_progress_ += anim_step_;
    if (anim_progress_ >= 1.0) {
      anim_progress_ = 1.0;
      animating_ = false;
      animation_timer_.Stop();
      // DO NOT Refresh() here if we have a completion event, otherwise we'll paint
      // with animating_=false BUT the old board state for 1 frame, causing a visual flash.
      // The completion event handler will update the board data and call Refresh().
      if (completion_event_) {
        GetParent()->GetEventHandler()->AddPendingEvent(*completion_event_);
        delete completion_event_;
        completion_event_ = nullptr;
      } else {
        Refresh();
      }
    } else {
      Refresh();
    }
  }

  Board* board_;
  int theme_index_ = 0;
  wxTimer animation_timer_;
  bool animating_ = false;
  Direction anim_dir_;
  int anim_tile_ = -1;
  double anim_progress_ = 0.0;
  double anim_step_ = 0.1;
  wxCommandEvent* completion_event_ = nullptr;
};

class SliderFrame : public wxFrame {
 public:
  SliderFrame() : wxFrame(nullptr, wxID_ANY, "Sliding Puzzle", wxDefaultPosition, wxSize(600, 700)) {
    board_ = std::make_unique<Board>(3);
    SetupMenu();
    SetupUI();
    
    Bind(wxEVT_MENU, &SliderFrame::OnNewGame, this, wxID_NEW);
    Bind(wxEVT_MENU, &SliderFrame::OnSaveGame, this, wxID_SAVE);
    Bind(wxEVT_MENU, &SliderFrame::OnLoadGame, this, wxID_OPEN);
    Bind(wxEVT_MENU, &SliderFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &SliderFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &SliderFrame::OnSize3, this, 10003);
    Bind(wxEVT_MENU, &SliderFrame::OnSize4, this, 10004);
    Bind(wxEVT_MENU, &SliderFrame::OnSize5, this, 10005);
    
    board_panel_->Bind(wxEVT_BUTTON, &SliderFrame::OnTileClicked, this);
    // Bind the move-completion event once here, not once per move
    Bind(wxEVT_COMMAND_MENU_SELECTED, &SliderFrame::OnMoveComplete, this, 30000);
    UpdateStatus();
  }

 private:
  void SetupMenu() {
    auto* fileMenu = new wxMenu;
    fileMenu->Append(wxID_NEW, "&New Game\tCtrl+N");
    fileMenu->Append(wxID_OPEN, "&Load Game\tCtrl+L");
    fileMenu->Append(wxID_SAVE, "&Save Game\tCtrl+S");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit");

    auto* sizeMenu = new wxMenu;
    sizeMenu->AppendRadioItem(10003, "3x3 Variation");
    sizeMenu->AppendRadioItem(10004, "4x4 (15 Puzzle)");
    sizeMenu->AppendRadioItem(10005, "5x5 Variation");

    auto* themeMenu = new wxMenu;
    for (int i = 0; i < static_cast<int>(kThemes.size()); ++i) {
      themeMenu->AppendRadioItem(20000 + i, kThemes[i].name);
      Bind(wxEVT_MENU, [this, i](wxCommandEvent&) { board_panel_->SetTheme(i); }, 20000 + i);
    }

    auto* helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&About");

    auto* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(sizeMenu, "&Size");
    menuBar->Append(themeMenu, "&Theme");
    menuBar->Append(helpMenu, "&Help");
    SetMenuBar(menuBar);
  }

  void SetupUI() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    board_panel_ = new BoardPanel(this, board_.get());
    mainSizer->Add(board_panel_, 1, wxEXPAND | wxALL, 10);

    auto* controlSizer = new wxBoxSizer(wxHORIZONTAL);
    scramble_btn_ = new wxButton(this, wxID_ANY, "Scramble");
    scramble_btn_->Bind(wxEVT_BUTTON, &SliderFrame::OnScramble, this);
    controlSizer->Add(scramble_btn_, 0, wxALL, 5);

    solve4_btn_ = new wxButton(this, wxID_ANY, "Solve 4 Steps");
    solve4_btn_->Bind(wxEVT_BUTTON, &SliderFrame::OnSolve4, this);
    controlSizer->Add(solve4_btn_, 0, wxALL, 5);

    solve_btn_ = new wxButton(this, wxID_ANY, "Solve");
    solve_btn_->Bind(wxEVT_BUTTON, &SliderFrame::OnSolve, this);
    controlSizer->Add(solve_btn_, 0, wxALL, 5);

    mainSizer->Add(controlSizer, 0, wxALIGN_CENTER | wxBOTTOM, 10);
    status_text_ = new wxStaticText(this, wxID_ANY, "Moves: 0 | Optimal: -");
    mainSizer->Add(status_text_, 0, wxALIGN_CENTER | wxBOTTOM, 10);
    SetSizer(mainSizer);
  }

  void OnNewGame(wxCommandEvent& /*event*/) {
    int size = board_->GetSize();
    board_ = std::make_unique<Board>(size);
    board_panel_->SetBoard(board_.get());
    optimal_moves_ = -1;
    UpdateStatus();
  }

  void OnSaveGame(wxCommandEvent& /*event*/) {
    wxFileDialog saveFileDialog(this, "Save Game State", "", "game.sav", "Save files (*.sav)|*.sav", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;
    std::ofstream os(saveFileDialog.GetPath().ToStdString());
    os << board_->GetState().Serialize();
  }

  void OnLoadGame(wxCommandEvent& /*event*/) {
    wxFileDialog openFileDialog(this, "Open Game State", "", "", "Save files (*.sav)|*.sav", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) return;
    std::ifstream is(openFileDialog.GetPath().ToStdString());
    std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    auto state = BoardState::Deserialize(content);
    if (state.IsValid()) {
      board_->SetState(state);
      board_panel_->Refresh();
      UpdateStatus();
    } else {
      wxMessageBox("Invalid save file", "Error", wxOK | wxICON_ERROR);
    }
  }

  void OnExit(wxCommandEvent& /*event*/) { Close(true); }

  void OnAbout(wxCommandEvent& /*event*/) {
    wxAboutDialogInfo info;
    info.SetName("Sliding Puzzle");
    info.SetVersion("1.0");
    info.SetDescription("A beautiful, cross-platform 15-puzzle game featuring dynamic visual themes\nand an algorithmic solver to find the optimal path in real-time.");
    info.SetCopyright(wxString::FromUTF8("\xc2\xa9 2026 George Taylor"));
    wxAboutBox(info);
  }

  void OnSize3(wxCommandEvent& /*event*/) { ChangeSize(3); }
  void OnSize4(wxCommandEvent& /*event*/) { ChangeSize(4); }
  void OnSize5(wxCommandEvent& /*event*/) { ChangeSize(5); }

  void OnMoveComplete(wxCommandEvent& /*event*/) {
    if (completion_callback_) completion_callback_();
  }

  void ChangeSize(int size) {
    board_ = std::make_unique<Board>(size);
    board_panel_->SetBoard(board_.get());
    optimal_moves_ = -1;
    UpdateStatus();
  }

  void OnTileClicked(wxCommandEvent& event) {
    int tile_val = event.GetInt();
    auto dir = board_->GetDirectionToMoveTile(tile_val);
    if (dir) {
      PerformMove(*dir, tile_val, 0.2);
    }
  }

  // Instant (no animation) move for testing / solver step-through.
  // Returns true if the tile moved successfully.
  bool InstantMove(int tile_val) {
    auto dir = board_->GetDirectionToMoveTile(tile_val);
    if (!dir) return false;
    board_->Move(*dir);
    board_panel_->Refresh();
    UpdateStatus();
    if (board_->IsSolved()) {
      PlaySoundEffect("complete.wav");
      wxMessageBox("Puzzle Solved!", "Congratulations", wxOK | wxICON_INFORMATION);
    }
    return true;
  }

  void PlaySoundEffect(const wxString& filename) {
    wxFileName fn(wxStandardPaths::Get().GetExecutablePath());
    fn.SetFullName(filename);
    wxString path = fn.GetFullPath();
    if (!wxFileExists(path)) {
      fn.AppendDir("resources");
      path = fn.GetFullPath();
    }
    if (wxFileExists(path)) {
      wxSound::Play(path, wxSOUND_ASYNC);
    }
  }

  void PerformMove(Direction dir, int tile_val, double duration, bool is_auto = false) {
    if (board_panel_->IsAnimating()) return;
    PlaySoundEffect("slide.wav");
    
    completion_callback_ = [this, dir, is_auto]() {
      board_->Move(dir);
      board_panel_->Refresh();  // repaint AFTER board state has changed
      UpdateStatus();
      if (!is_scrambling_ && board_->IsSolved()) {
        PlaySoundEffect("complete.wav");
        wxMessageBox("Puzzle Solved!", "Congratulations", wxOK | wxICON_INFORMATION);
      }
      if (is_auto && !auto_moves_.empty()) {
        ProcessNextAutoMove();
      } else if (is_auto && auto_moves_.empty() && is_scrambling_) {
        is_scrambling_ = false;
        board_->ResetMoveCount();
        UpdateStatus();
      }
    };
    // One StartAnimation call, passing the completion event so the panel fires it when done
    auto* evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, 30000);
    board_panel_->StartAnimation(dir, tile_val, duration, evt);
  }

  void OnScramble(wxCommandEvent& /*event*/) {
    if (board_panel_->IsAnimating()) return;
    auto moves = Scrambler::Scramble(*board_, 30);
    int size = board_->GetSize();
    board_ = std::make_unique<Board>(size);
    board_panel_->SetBoard(board_.get()); // Fix dangling pointer!
    auto_moves_ = moves;
    auto_duration_ = 0.05; // Slightly faster for scrambling
    is_scrambling_ = true;
    
    Board temp_board = *board_;
    for(auto d : moves) temp_board.Move(d);
    auto sol = Solver::Solve(temp_board.GetState());
    optimal_moves_ = sol.success ? static_cast<int>(sol.moves.size()) : -1;
    ProcessNextAutoMove();
  }

  void OnSolve4(wxCommandEvent& /*event*/) {
    if (board_panel_->IsAnimating()) return;
    auto sol = Solver::SolveNSteps(board_->GetState(), 4);
    if (sol.success) {
      auto_moves_ = sol.moves;
      auto_duration_ = 0.5;
      ProcessNextAutoMove();
    }
  }

  void OnSolve(wxCommandEvent& /*event*/) {
    if (board_panel_->IsAnimating()) return;
    auto sol = Solver::Solve(board_->GetState());
    if (sol.success) {
      auto_moves_ = sol.moves;
      auto_duration_ = 0.5;
      ProcessNextAutoMove();
    } else {
      wxMessageBox("Solver could not find a solution in reasonable time.", "Info");
    }
  }

  void ProcessNextAutoMove() {
    if (auto_moves_.empty() || !board_) return;
    Direction dir = auto_moves_.front();
    auto_moves_.erase(auto_moves_.begin());
    
    int size = board_->GetSize();
    int empty_pos = board_->GetState().GetEmptyPos();
    if (empty_pos < 0 || empty_pos >= static_cast<int>(board_->GetState().GetTiles().size())) return;

    int r = empty_pos / size;
    int c = empty_pos % size;
    int tr = r, tc = c;
    switch(dir) {
        case Direction::kUp: tr--; break;
        case Direction::kDown: tr++; break;
        case Direction::kLeft: tc--; break;
        case Direction::kRight: tc++; break;
    }
    
    if (tr >= 0 && tr < size && tc >= 0 && tc < size) {
      int tile_pos = tr * size + tc;
      const auto& tiles = board_->GetState().GetTiles();
      if (tile_pos >= 0 && tile_pos < static_cast<int>(tiles.size())) {
        int tile_val = tiles[tile_pos];
        PerformMove(dir, tile_val, auto_duration_, true);
      }
    } else {
      if (!auto_moves_.empty()) ProcessNextAutoMove();
    }
  }

  void UpdateStatus() {
    wxString opt = optimal_moves_ >= 0 ? wxString::Format("%d", optimal_moves_) : wxString("?");
    status_text_->SetLabel(wxString::Format("Moves: %d | Optimal: %s", board_->GetMoveCount(), opt));
  }

  std::unique_ptr<Board> board_;
  BoardPanel* board_panel_ = nullptr;
  wxStaticText* status_text_ = nullptr;
  wxButton* scramble_btn_ = nullptr;
  wxButton* solve4_btn_ = nullptr;
  wxButton* solve_btn_ = nullptr;
  std::vector<Direction> auto_moves_;
  double auto_duration_ = 0.5;
  int optimal_moves_ = -1;
  std::function<void()> completion_callback_;
  bool is_scrambling_ = false;
};

class SliderApp : public wxApp {
 public:
  bool OnInit() override {
    auto* frame = new SliderFrame();
    frame->Show(true);
    return true;
  }
};

wxIMPLEMENT_APP(SliderApp);
