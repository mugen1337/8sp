
# include <Siv3D.hpp> // OpenSiv3D v0.4.0
# include <queue>
#include<Windows.h>
using namespace std;

int board[3][3] = { {1,2,3},{4,5,6},{7,9,8} };
int correct_x[10] = { -1,0,1,2,0,1,2,0,1,2 };
int correct_y[10] = { -1,0,0,0,1,1,1,2,2,2 };
//shuffle回数。2~8くらいから上はそんな変わらないよ
int timesOfShuffle = 10;

//空マスの位置をペアで返す関数
pair<int, int> posEmp(int b[3][3]) {
	for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) if (b[i][j] == 9) return (make_pair(i, j));
}

// スタートからゴールまでの交換の回数の偶奇=空きマスのゴールまでの最短距離の偶奇なら盤面はクリア可能
bool can() {
	//盤面配列を他の配列bにコピーしてからbがクリア可能かをチェックする
	int b[3][3];
	for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) b[i][j] = board[i][j]; //copy
	//空マスのゴールまでの距離とクリア盤面までのスワップ回数をそれぞれ調べる
	int emptyDis;
	for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) if (b[i][j] == 9) emptyDis = 4 - i - j;

	//正しい位置にnumがなければswapしてswap数をカウント
	int countSwap = 0;
	for (int num = 1; num < 9; num++)for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++)
		if (b[i][j] == num) {
			if (correct_x[num] == j && correct_y[num] == i) continue;
			countSwap++;
			swap(b[i][j], b[correct_y[num]][correct_x[num]]);
		}
	return ((countSwap % 2) == (emptyDis % 2));
}


//クリア可能になるまで盤面のシャッフル
void shuffle() {
	for (int time = 0; time < timesOfShuffle; time++) {
		int x1 = rand() % 3, y1 = rand() % 3,
			x2 = rand() % 3, y2 = rand() % 3;
		swap(board[x1][y1], board[x2][y2]);
	}
	if (!can()) shuffle();
}

//クリアかどうかの判定
bool judge(int b[3][3]) {
	if (b[2][2] != 9) return false;
	for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) if (i != correct_y[b[i][j]] || j != correct_x[b[i][j]]) return false;
	return true;
}

struct boardCost {
	//盤面b
	int b[3][3];
	//評価関数
	//ゴールについたとき、この値はゴールにたどり着くまでのコストになる
	//この値の更新はf=f(now)-h(now)+h(new)+1(操作分)
	int f;
	//このマスのゴールまでのマンハッタン距離
	int m;
	//今までの手順
	string way;
	//priorityqueueから小さいのが出てきて欲しいから通常の演算子と逆に<を指定している。よくないかな。。。
	bool operator<(const boardCost& x) const { return f > x.f; }
};


//ヒューリスティック関数 各点の答え盤面までのマンハッタン距離
int h(int b[3][3]) {
	int ret = 0;
	for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) ret += abs(i - correct_y[b[i][j]]) + abs(j - correct_x[b[i][j]]);
	return ret;
}

//A*探索 ゴールまでの手順を探索してその手順を返す
string aStar(int b[3][3]) {
	//評価関数の値の小さいものから出てくるキュー。オープンリストってやつ。リストじゃないけど
	priority_queue<boardCost> que;
	//スタート盤面をqueへ
	boardCost st;
	for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) st.b[i][j] = board[i][j];//copy
	st.f = h(board); st.m = st.f; st.way = "";
	que.push(st);

	//すでに探索した盤面は探索する必要がない
	set<string> searched;
	string sts;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			sts += to_string(st.b[i][j]);
		}
	}
	searched.insert(sts);

	//ゴールにまで探索
	while (!que.empty()) {
		boardCost now = que.top();
		que.pop();
		//マンハッタン距離が0ならクリアだからそこで終わり
		if (now.m == 0) { return now.way; }
		pair<int, int> posEmpty = posEmp(now.b);
		if (posEmpty.second != 0) {
			//空マスが左端じゃない
			boardCost tmp;
			for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) tmp.b[i][j] = now.b[i][j];//copy
			swap(tmp.b[posEmpty.first][posEmpty.second], tmp.b[posEmpty.first][posEmpty.second - 1]);
			//盤面の状態をstringにして保持
			string tmps = "";
			for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) tmps += to_string(tmp.b[i][j]);
			//その盤面の状態がすでに探索済みだったらもう一度探索する必要はない
			if (searched.count(tmps) == 0) {
				//次の盤面をｷｭｰへつっこむ
				searched.insert(tmps);
				tmp.m = h(tmp.b);
				tmp.f = now.f - now.m + tmp.m + 1;
				tmp.way = now.way + "R";
				que.push(tmp);
			}
		}
		//以下これの繰り返し
		if (posEmpty.second != 2) {
			//空マスが右端じゃない
			boardCost tmp;
			for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) tmp.b[i][j] = now.b[i][j];//copy
			swap(tmp.b[posEmpty.first][posEmpty.second], tmp.b[posEmpty.first][posEmpty.second + 1]);
			string tmps = "";
			for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) tmps += to_string(tmp.b[i][j]);
			if (searched.count(tmps) == 0) {
				searched.insert(tmps);
				tmp.m = h(tmp.b);
				tmp.f = now.f - now.m + tmp.m + 1;
				tmp.way = now.way + "L";
				que.push(tmp);
			}
		}
		if (posEmpty.first != 0) {
			//空マスが上じゃない
			boardCost tmp;
			for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) tmp.b[i][j] = now.b[i][j];//copy
			swap(tmp.b[posEmpty.first][posEmpty.second], tmp.b[posEmpty.first - 1][posEmpty.second]);
			string tmps = "";
			for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) tmps += to_string(tmp.b[i][j]);
			if (searched.count(tmps) == 0) {
				searched.insert(tmps);
				tmp.m = h(tmp.b);
				tmp.f = now.f - now.m + tmp.m + 1;
				tmp.way = now.way + "D";
				que.push(tmp);
			}
		}
		if (posEmpty.first != 2) {
			//空マスが下じゃない
			boardCost tmp;
			for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) tmp.b[i][j] = now.b[i][j];//copy
			swap(tmp.b[posEmpty.first][posEmpty.second], tmp.b[posEmpty.first + 1][posEmpty.second]);
			string tmps = "";
			for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++) tmps += to_string(tmp.b[i][j]);
			if (searched.count(tmps) == 0) {
				searched.insert(tmps);
				tmp.m = h(tmp.b);
				tmp.f = now.f - now.m + tmp.m + 1;
				tmp.way = now.way + "U";
				que.push(tmp);
			}
		}
	}
	return "error";
}

void Main()
{
	const Image image = Dialog::OpenImage();
	const Texture texture(image.squareClipped(), TextureDesc::Mipped);
	TextureRegion pieces[10];
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			pieces[i * 3 + j+1 ] = texture.uv(0.33 * j, 0.33 * i, 0.33, 0.33);
		}
	}

	srand((unsigned)time(NULL));
	shuffle();
	// 背景
	Scene::SetBackground(ColorF(0.8, 0.9, 1.0));
	constexpr Point offset(60, 40);
	constexpr int32 cellSize = 100;

	const Font font(70);

	//数字に対応するテクスチャ後々画像をこれらに対応させればいい？
	const Texture nums[10] = { Texture(Emoji(U"0⃣")),Texture(Emoji(U"1⃣")) ,Texture(Emoji(U"2⃣")),
	Texture(Emoji(U"3⃣")), Texture(Emoji(U"4⃣")) ,Texture(Emoji(U"5⃣")) ,Texture(Emoji(U"6⃣"))
	,Texture(Emoji(U"7⃣")) ,Texture(Emoji(U"8⃣")) ,Texture(Emoji(U"□")) };
	
	bool Lock = false;
	string ope;
	int ind = 0;

	while (System::Update())
	{
		texture.resized(180).draw(offset.x + cellSize * 3 + 40, offset.y)
			.drawFrame(0, 3, ColorF(0.3, 0.5, 0.7));

		Rect(offset, 3 * cellSize).drawShadow(Vec2(0, 2), 12, 8).draw(ColorF(0.25))
			.drawFrame(0, 8, ColorF(0.3, 0.5, 0.7));

		
		//盤面の描写→キーを押されることによる盤面の更新→盤面の描写
		//盤面の描写
		
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				if (board[i][j] == 9) continue;
				pieces[board[i][j]].resized(100).draw(offset.x + j * cellSize +1 , 2+offset.y + i * cellSize);
			}
		}
		if (judge(board)) {
			font(U"CONGRATURATION").drawAt(Scene::Center(), Palette::Crimson);
			Lock = true;
		}

		
		if (!Lock) {
			// [A] キーが押されたら
			if (KeyL.down() || KeyLeft.down())
			{
				pair<int, int> posEmpty = posEmp(board);
				if (posEmpty.second != 2)
					swap(board[posEmpty.first][posEmpty.second], board[posEmpty.first][posEmpty.second + 1]);
			}
			if (KeyR.down() || KeyRight.down())
			{
				pair<int, int> posEmpty = posEmp(board);
				if (posEmpty.second != 0)
					swap(board[posEmpty.first][posEmpty.second], board[posEmpty.first][posEmpty.second - 1]);
			}
			if (KeyD.down() || KeyDown.down())
			{
				pair<int, int> posEmpty = posEmp(board);
				if (posEmpty.first != 0)
					swap(board[posEmpty.first][posEmpty.second], board[posEmpty.first - 1][posEmpty.second]);
			}
			if (KeyU.down() || KeyUp.down())
			{
				pair<int, int> posEmpty = posEmp(board);
				if (posEmpty.first != 2)
					swap(board[posEmpty.first][posEmpty.second], board[posEmpty.first + 1][posEmpty.second]);
			}

			if (SimpleGUI::Button(U"A*", Vec2(520, 300), 200)) {
				//このボタンが押されたら今の盤面からA*探索をしてすべてを終わらせる
				//探索中は操作を受け付けないようにLockをかける
				Lock = true;
			}
		}
		else {
			Sleep(400);
			if(ope.size()==0) ope = aStar(board);
				pair<int, int> posEmpty = posEmp(board);
				if (ind>ope.size()) {
					none;
				}
				else if (ope[ind] == 'L') {
					swap(board[posEmpty.first][posEmpty.second], board[posEmpty.first][posEmpty.second + 1]);
					ind++;
				}
				else if (ope[ind] == 'R') {
					swap(board[posEmpty.first][posEmpty.second], board[posEmpty.first][posEmpty.second - 1]);
					ind++;
				}
				else if (ope[ind] == 'D') {
					swap(board[posEmpty.first][posEmpty.second], board[posEmpty.first - 1][posEmpty.second]);
					ind++;
				}
				else if (ope[ind] == 'U') {
					swap(board[posEmpty.first][posEmpty.second], board[posEmpty.first + 1][posEmpty.second]);
					ind++;
				}
		}
	}
}
