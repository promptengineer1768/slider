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

wxString ResolveSoundPath(const wxString& filename) {
  wxFileName exe_path(wxStandardPaths::Get().GetExecutablePath());
  exe_path.SetFullName(filename);
  if (exe_path.FileExists()) {
    return exe_path.GetFullPath();
  }

  wxFileName resource_path(wxStandardPaths::Get().GetExecutablePath());
  resource_path.AppendDir("resources");
  resource_path.SetFullName(filename);
  if (resource_path.FileExists()) {
    return resource_path.GetFullPath();
  }

  wxFileName cwd_path(wxGetCwd(), filename);
  if (cwd_path.FileExists()) {
    return cwd_path.GetFullPath();
  }

  wxFileName cwd_resource_path(wxGetCwd(), filename);
  cwd_resource_path.AppendDir("resources");
  if (cwd_resource_path.FileExists()) {
    return cwd_resource_path.GetFullPath();
  }

  return {};
}

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

    // Setup high-quality graphics context
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (!gc) return;

    // 1. Draw Board Container (The Frame)
    // We create a "well" that the tiles sit inside of.
    wxRect board_rect = client_rect;
    board_rect.Deflate(10);
    
    // Outer shadow for the board well
    gc->SetBrush(gc->CreateBrush(wxBrush(wxColour(0, 0, 0, 40))));
    gc->DrawRoundedRectangle(board_rect.x + 2, board_rect.y + 2, board_rect.width, board_rect.height, 15);
    
    // Board well background
    wxGraphicsBrush bg_brush = gc->CreateLinearGradientBrush(
        board_rect.x, board_rect.y, board_rect.x, board_rect.y + board_rect.height,
        theme.bg_secondary.ChangeLightness(90), theme.bg_secondary.ChangeLightness(110));
    gc->SetBrush(bg_brush);
    gc->SetPen(wxPen(theme.bg_primary.ChangeLightness(80), 2));
    gc->DrawRoundedRectangle(board_rect.x, board_rect.y, board_rect.width, board_rect.height, 15);

    // 2. Adjust cell sizes for the inner board area
    int inner_w = board_rect.width;
    int inner_h = board_rect.height;
    int cell_w = inner_w / size;
    int cell_h = inner_h / size;

    const auto& tiles = board_->GetState().GetTiles();

    for (int i = 0; i < static_cast<int>(tiles.size()); ++i) {
      if (tiles[i] == 0) continue;

      int row = i / size;
      int col = i % size;

      double off_x = 0, off_y = 0;
      if (animating_ && tiles[i] == anim_tile_) {
        switch (anim_dir_) {
          case Direction::kUp:    off_y = +anim_progress_ * cell_h; break;
          case Direction::kDown:  off_y = -anim_progress_ * cell_h; break;
          case Direction::kLeft:  off_x = +anim_progress_ * cell_w; break;
          case Direction::kRight: off_x = -anim_progress_ * cell_w; break;
        }
      }

      // Tile rectangle relative to the board well
      wxRect rect(board_rect.x + col * cell_w + static_cast<int>(off_x), 
                  board_rect.y + row * cell_h + static_cast<int>(off_y), 
                  cell_w, cell_h);
      rect.Deflate(6);

      // A. Shadow (sharper, matched rounding)
      gc->SetBrush(gc->CreateBrush(wxBrush(wxColour(0, 0, 0, 60))));
      gc->SetPen(*wxTRANSPARENT_PEN);
      gc->DrawRoundedRectangle(rect.x + 2, rect.y + 4, rect.width, rect.height, 12);

      // B. Body (Cushioned Gradient)
      wxGraphicsPath tile_path = gc->CreatePath();
      tile_path.AddRoundedRectangle(rect.x, rect.y, rect.width, rect.height, 12);
      
      wxGraphicsBrush tile_brush = gc->CreateLinearGradientBrush(
          rect.x, rect.y, rect.x, rect.y + rect.height,
          theme.tile_highlight, theme.tile_shadow);
      gc->SetBrush(tile_brush);
      gc->DrawPath(tile_path);

      // C. Bevels (Raised 3D Look)
      // Top highlight
      gc->SetPen(wxPen(wxColour(255, 255, 255, 120), 3));
      gc->StrokeLine(rect.x + 12, rect.y + 2, rect.x + rect.width - 12, rect.y + 2);
      // Left highlight
      gc->StrokeLine(rect.x + 2, rect.y + 12, rect.x + 2, rect.y + rect.height - 12);
      
      // Bottom/Right darkened bevel
      gc->SetPen(wxPen(wxColour(0, 0, 0, 40), 2));
      gc->StrokeLine(rect.x + 12, rect.y + rect.height - 1, rect.x + rect.width - 12, rect.y + rect.height - 1);
      gc->StrokeLine(rect.x + rect.width - 1, rect.y + 12, rect.x + rect.width - 1, rect.y + rect.height - 12);

      // D. Inner sheen (Subtle 3D rounding effect)
      wxGraphicsPath sheen = gc->CreatePath();
      sheen.AddRoundedRectangle(rect.x + 2, rect.y + 2, rect.width - 4, rect.height / 2, 10);
      gc->SetBrush(gc->CreateLinearGradientBrush(
          rect.x, rect.y, rect.x, rect.y + rect.height / 2,
          wxColour(255, 255, 255, 60), wxColour(255, 255, 255, 0)));
      gc->SetPen(*wxTRANSPARENT_PEN);
      gc->DrawPath(sheen);

        // E. Number
        if (cell_h > 0) {
          wxFont font(cell_h / 3, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
          wxString label = wxString::Format("%d", tiles[i]);
          
          double tw, th, td, tl;
          gc->SetFont(font, theme.text_color);
          gc->GetTextExtent(label, &tw, &th, &td, &tl);
          
          double tx = rect.x + (rect.width - tw) / 2.0;
          double ty = rect.y + (rect.height - th) / 2.0;

          // 1. Drop Shadow (Soft dark underlay)
          gc->SetFont(font, wxColour(0, 0, 0, 100));
          gc->DrawText(label, tx + 1.5, ty + 1.5);

          // 2. Glow Aura (Multiple semi-transparent white passes)
          gc->SetFont(font, wxColour(255, 255, 255, 40));
          gc->DrawText(label, tx - 1, ty);
          gc->DrawText(label, tx + 1, ty);
          gc->DrawText(label, tx, ty - 1);
          gc->DrawText(label, tx, ty + 1);

          // 3. Main Text (Sharp colored overlay)
          gc->SetFont(font, theme.text_color);
          gc->DrawText(label, tx, ty);
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
    
    sound_stop_timer_.Bind(wxEVT_TIMER, [this](wxTimerEvent&) {
      wxSound::Stop();
    });

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
    auto_play_slide_sound_ = true;
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
    auto_play_slide_sound_ = true;
    UpdateStatus();
  }

  void OnTileClicked(wxCommandEvent& event) {
    int tile_val = event.GetInt();
    auto dir = board_->GetDirectionToMoveTile(tile_val);
    if (dir) {
      PerformMove(*dir, tile_val, 0.2, false, true);
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
      PlaySoundEffect("complete.wav", true);
      wxMessageBox("Puzzle Solved!", "Congratulations", wxOK | wxICON_INFORMATION);
    }
    return true;
  }

  void PlaySoundEffect(const wxString& filename, bool is_completion = false) {
    wxString path = ResolveSoundPath(filename);
    if (!path.empty()) {
      wxSound::Play(path, wxSOUND_ASYNC);
      if (is_completion) {
        sound_stop_timer_.Start(2000, wxTIMER_ONE_SHOT);
      }
    }
  }

  void PerformMove(Direction dir, int tile_val, double duration, bool is_auto = false,
                   bool play_slide_sound = true) {
    if (board_panel_->IsAnimating()) return;
    if (play_slide_sound) {
      PlaySoundEffect("slide.wav");
    }
    
    completion_callback_ = [this, dir, is_auto]() {
      board_->Move(dir);
      board_panel_->Refresh();  // repaint AFTER board state has changed
      UpdateStatus();
      if (!is_scrambling_ && board_->IsSolved()) {
        PlaySoundEffect("complete.wav", true);
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
    int scramble_steps = board_->GetSize() == 5 ? 60 : 30;
    auto moves = Scrambler::Scramble(*board_, scramble_steps);
    int size = board_->GetSize();
    board_ = std::make_unique<Board>(size);
    board_panel_->SetBoard(board_.get()); // Fix dangling pointer!
    auto_moves_ = moves;
    auto_duration_ = 0.05; // Slightly faster for scrambling
    auto_play_slide_sound_ = false;
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
      auto_play_slide_sound_ = true;
      is_scrambling_ = false; 
      ProcessNextAutoMove();
    }
  }

  void OnSolve(wxCommandEvent& /*event*/) {
    if (board_panel_->IsAnimating()) return;
    auto sol = Solver::Solve(board_->GetState());
    if (sol.success) {
      auto_moves_ = sol.moves;
      auto_duration_ = 0.5;
      auto_play_slide_sound_ = true;
      is_scrambling_ = false;
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
        PerformMove(dir, tile_val, auto_duration_, true,
                    auto_play_slide_sound_ && !is_scrambling_);
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
  bool auto_play_slide_sound_ = true;
  wxTimer sound_stop_timer_;
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
