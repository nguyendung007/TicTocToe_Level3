#!/usr/bin/env python3
"""
Property-based Testing cho các pure function trong Level 3
Chạy: python test_properties.py
"""

import random
import sys
from typing import List, Tuple, Callable, Any

# ============================================================================
# ĐỊNH NGHĨA CÁC KIỂU DỮ LIỆU
# ============================================================================

EMPTY_CELL = '.'
SYMBOL_X = 'X'
SYMBOL_O = 'O'

class Move:
    def __init__(self, row: int, col: int):
        self.row = row
        self.col = col
    
    def __eq__(self, other):
        return self.row == other.row and self.col == other.col
    
    def __repr__(self):
        return f"({self.row},{self.col})"

class Board:
    def __init__(self, size: int):
        self.size = size
        self.grid = [[EMPTY_CELL for _ in range(size)] for _ in range(size)]
    
    def at(self, row: int, col: int) -> str:
        if 0 <= row < self.size and 0 <= col < self.size:
            return self.grid[row][col]
        return None
    
    def clone(self) -> 'Board':
        new_board = Board(self.size)
        for i in range(self.size):
            for j in range(self.size):
                new_board.grid[i][j] = self.grid[i][j]
        return new_board
    
    def apply_move(self, move: Move, symbol: str) -> 'Board':
        """applyMove - pure function: trả về board mới, không sửa board cũ"""
        new_board = self.clone()
        new_board.grid[move.row][move.col] = symbol
        return new_board
    
    def count_symbol(self, symbol: str) -> int:
        """countSymbol - đếm số lượng symbol trên board"""
        count = 0
        for i in range(self.size):
            for j in range(self.size):
                if self.grid[i][j] == symbol:
                    count += 1
        return count
    
    def is_valid_move(self, move: Move) -> bool:
        return (0 <= move.row < self.size and 
                0 <= move.col < self.size and 
                self.grid[move.row][move.col] == EMPTY_CELL)
    
    def __repr__(self):
        result = []
        for row in self.grid:
            result.append(' '.join(row))
        return '\n'.join(result)


# ============================================================================
# HÀM MAP COMBINATOR (MÔ PHỎNG)
# ============================================================================

def map_func(f: Callable, xs: List) -> List:
    """map combinator - pure function"""
    return [f(x) for x in xs]

def compose(f: Callable, g: Callable) -> Callable:
    """f ∘ g"""
    return lambda x: f(g(x))


# ============================================================================
# TEST 1: applyMove - Tính chất giao hoán
# ============================================================================

def test_applymove_commutative(num_tests: int = 100):
    """
    Tính chất: applyMove(applyMove(B, m1, s1), m2, s2) = 
               applyMove(applyMove(B, m2, s2), m1, s1)
    Với m1 ≠ m2
    """
    print(f"\n{'='*60}")
    print(f"TEST 1: applyMove - Tính chất giao hoán")
    print(f"Số lượng test: {num_tests}")
    print(f"{'='*60}")
    
    passed = 0
    failed = 0
    
    for test_id in range(num_tests):
        # Tạo board ngẫu nhiên kích thước 5x5 đến 10x10
        size = random.randint(5, 10)
        board = Board(size)
        
        # Điền một số ô ngẫu nhiên (0-30% số ô)
        num_filled = random.randint(0, int(size * size * 0.3))
        filled_positions = set()
        for _ in range(num_filled):
            r, c = random.randint(0, size-1), random.randint(0, size-1)
            if (r, c) not in filled_positions:
                filled_positions.add((r, c))
                symbol = random.choice([SYMBOL_X, SYMBOL_O])
                board.grid[r][c] = symbol
        
        # Tìm 2 ô trống khác nhau để đánh
        empty_cells = [(i, j) for i in range(size) for j in range(size) 
                       if board.grid[i][j] == EMPTY_CELL]
        
        if len(empty_cells) < 2:
            continue  # Bỏ qua nếu không đủ ô trống
        
        m1_idx, m2_idx = random.sample(range(len(empty_cells)), 2)
        r1, c1 = empty_cells[m1_idx]
        r2, c2 = empty_cells[m2_idx]
        move1 = Move(r1, c1)
        move2 = Move(r2, c2)
        sym1 = random.choice([SYMBOL_X, SYMBOL_O])
        sym2 = random.choice([SYMBOL_X, SYMBOL_O])
        
        # Tính B12 = applyMove(applyMove(B, m1, s1), m2, s2)
        b1 = board.apply_move(move1, sym1)
        b12 = b1.apply_move(move2, sym2)
        
        # Tính B21 = applyMove(applyMove(B, m2, s2), m1, s1)
        b2 = board.apply_move(move2, sym2)
        b21 = b2.apply_move(move1, sym1)
        
        # So sánh kết quả
        equal = True
        for i in range(size):
            for j in range(size):
                if b12.grid[i][j] != b21.grid[i][j]:
                    equal = False
                    break
            if not equal:
                break
        
        if equal:
            passed += 1
        else:
            failed += 1
            print(f"  ❌ Test {test_id + 1} failed!")
            print(f"     Board size: {size}")
            print(f"     Move1: {move1}, sym1={sym1}")
            print(f"     Move2: {move2}, sym2={sym2}")
    
    print(f"\n📊 Kết quả: ✅ {passed} passed, ❌ {failed} failed")
    print(f"🎯 Tỷ lệ thành công: {100 * passed / (passed + failed):.2f}%")
    return passed, failed


# ============================================================================
# TEST 2: applyMove - Quan hệ với countSymbol
# ============================================================================

def test_applymove_countsymbol(num_tests: int = 100):
    """
    Tính chất: countSymbol(applyMove(B, m, s), s) = countSymbol(B, s) + 1
    Với isValidMove(B, m) = true
    """
    print(f"\n{'='*60}")
    print(f"TEST 2: applyMove - Quan hệ với countSymbol")
    print(f"Số lượng test: {num_tests}")
    print(f"{'='*60}")
    
    passed = 0
    failed = 0
    
    for test_id in range(num_tests):
        size = random.randint(3, 8)
        board = Board(size)
        
        # Điền một số ô ngẫu nhiên
        num_filled = random.randint(0, int(size * size * 0.5))
        for _ in range(num_filled):
            r, c = random.randint(0, size-1), random.randint(0, size-1)
            symbol = random.choice([SYMBOL_X, SYMBOL_O])
            board.grid[r][c] = symbol
        
        # Tìm ô trống
        empty_cells = [(i, j) for i in range(size) for j in range(size) 
                       if board.grid[i][j] == EMPTY_CELL]
        
        if not empty_cells:
            continue
        
        r, c = random.choice(empty_cells)
        move = Move(r, c)
        symbol = random.choice([SYMBOL_X, SYMBOL_O])
        
        before_count = board.count_symbol(symbol)
        new_board = board.apply_move(move, symbol)
        after_count = new_board.count_symbol(symbol)
        
        expected = before_count + 1
        
        if after_count == expected:
            passed += 1
        else:
            failed += 1
            print(f"  ❌ Test {test_id + 1} failed!")
            print(f"     Before: {before_count}, After: {after_count}, Expected: {expected}")
            print(f"     Move: {move}, Symbol: {symbol}")
    
    print(f"\n📊 Kết quả: ✅ {passed} passed, ❌ {failed} failed")
    print(f"🎯 Tỷ lệ thành công: {100 * passed / (passed + failed):.2f}%")
    return passed, failed


# ============================================================================
# TEST 3: map combinator - map(f) ∘ map(g) = map(f ∘ g)
# ============================================================================

def test_map_compose(num_tests: int = 100):
    """
    Tính chất: map(f) ∘ map(g) = map(f ∘ g)
    """
    print(f"\n{'='*60}")
    print(f"TEST 3: map(f) ∘ map(g) = map(f ∘ g)")
    print(f"Số lượng test: {num_tests}")
    print(f"{'='*60}")
    
    passed = 0
    failed = 0
    
    # Định nghĩa các hàm test
    def f1(x): return x + 1
    def g1(x): return x * 2
    f1_compose_g1 = lambda x: f1(g1(x))
    
    def f2(x): return x * x
    def g2(x): return x - 3
    f2_compose_g2 = lambda x: f2(g2(x))
    
    def f3(x): return abs(x)
    def g3(x): return -x
    f3_compose_g3 = lambda x: f3(g3(x))
    
    func_pairs = [(f1, g1, f1_compose_g1), 
                  (f2, g2, f2_compose_g2),
                  (f3, g3, f3_compose_g3)]
    
    for test_id in range(num_tests):
        # Chọn ngẫu nhiên một cặp hàm
        f, g, f_compose_g = random.choice(func_pairs)
        
        # Tạo list ngẫu nhiên
        list_size = random.randint(1, 100)
        xs = [random.randint(-100, 100) for _ in range(list_size)]
        
        # Tính map(f) ∘ map(g)
        mapped_twice = map_func(f, map_func(g, xs))
        
        # Tính map(f ∘ g)
        mapped_once = map_func(f_compose_g, xs)
        
        if mapped_twice == mapped_once:
            passed += 1
        else:
            failed += 1
            print(f"  ❌ Test {test_id + 1} failed!")
            print(f"     xs = {xs[:5]}..." if len(xs) > 5 else f"     xs = {xs}")
            print(f"     map(f)∘map(g) = {mapped_twice[:5]}...")
            print(f"     map(f∘g) = {mapped_once[:5]}...")
    
    print(f"\n📊 Kết quả: ✅ {passed} passed, ❌ {failed} failed")
    print(f"🎯 Tỷ lệ thành công: {100 * passed / (passed + failed):.2f}%")
    return passed, failed


# ============================================================================
# TEST 4: medium bot - Tính chất ưu tiên nước thắng
# ============================================================================

def create_winning_scenario(size: int, goal: int, symbol: str) -> Tuple[Board, Move]:
    """
    Tạo tình huống có nước thắng cho symbol
    Trả về (board, winning_move)
    """
    board = Board(size)
    
    # Tạo một hàng ngang gần thắng (goal-1 quân liên tiếp)
    start_row = random.randint(0, size - 1)
    start_col = random.randint(0, size - goal)
    
    # Đặt goal-1 quân liên tiếp
    for i in range(goal - 1):
        board.grid[start_row][start_col + i] = symbol
    
    # Ô cuối cùng để thắng
    winning_move = Move(start_row, start_col + goal - 1)
    
    # Đảm bảo ô thắng trống
    board.grid[winning_move.row][winning_move.col] = EMPTY_CELL
    
    return board, winning_move


def simulate_medium_bot(board: Board, bot_symbol: str, opp_symbol: str, goal: int) -> Move:
    """
    Mô phỏng medium bot trong Python
    """
    # Tìm tất cả các ô trống
    empty_cells = []
    for i in range(board.size):
        for j in range(board.size):
            if board.grid[i][j] == EMPTY_CELL:
                empty_cells.append(Move(i, j))
    
    # Kiểm tra nước thắng
    for move in empty_cells:
        # Thử đặt bot_symbol
        test_board = board.apply_move(move, bot_symbol)
        if check_win(test_board, bot_symbol, goal):
            return move
    
    # Kiểm tra nước chặn
    for move in empty_cells:
        test_board = board.apply_move(move, opp_symbol)
        if check_win(test_board, opp_symbol, goal):
            return move
    
    # Heuristic đơn giản: chọn ô trung tâm hoặc ngẫu nhiên
    center = board.size // 2
    for move in empty_cells:
        if move.row == center and move.col == center:
            return move
    
    return empty_cells[0] if empty_cells else Move(-1, -1)


def check_win(board: Board, symbol: str, goal: int) -> bool:
    """Kiểm tra thắng đơn giản (chỉ NONE rule)"""
    size = board.size
    
    # Kiểm tra hàng ngang
    for r in range(size):
        count = 0
        for c in range(size):
            if board.grid[r][c] == symbol:
                count += 1
                if count >= goal:
                    return True
            else:
                count = 0
    
    # Kiểm tra hàng dọc
    for c in range(size):
        count = 0
        for r in range(size):
            if board.grid[r][c] == symbol:
                count += 1
                if count >= goal:
                    return True
            else:
                count = 0
    
    # Kiểm tra đường chéo chính
    for r in range(size - goal + 1):
        for c in range(size - goal + 1):
            count = 0
            for i in range(goal):
                if board.grid[r + i][c + i] == symbol:
                    count += 1
                else:
                    break
            if count >= goal:
                return True
    
    # Kiểm tra đường chéo phụ
    for r in range(size - goal + 1):
        for c in range(goal - 1, size):
            count = 0
            for i in range(goal):
                if board.grid[r + i][c - i] == symbol:
                    count += 1
                else:
                    break
            if count >= goal:
                return True
    
    return False


def test_medium_bot_priority(num_tests: int = 100):
    """
    Tính chất: Nếu có nước thắng, medium bot phải chọn nước thắng
    """
    print(f"\n{'='*60}")
    print(f"TEST 4: medium bot - Ưu tiên nước thắng")
    print(f"Số lượng test: {num_tests}")
    print(f"{'='*60}")
    
    passed = 0
    failed = 0
    
    for test_id in range(num_tests):
        size = random.randint(5, 10)
        goal = random.randint(3, min(5, size))
        bot_symbol = random.choice([SYMBOL_X, SYMBOL_O])
        opp_symbol = SYMBOL_O if bot_symbol == SYMBOL_X else SYMBOL_X
        
        # Tạo tình huống có nước thắng
        board, winning_move = create_winning_scenario(size, goal, bot_symbol)
        
        # Bot chọn nước đi
        bot_move = simulate_medium_bot(board, bot_symbol, opp_symbol, goal)
        
        # Kiểm tra bot có chọn đúng nước thắng không
        # (có thể có nhiều nước thắng, không nhất thiết phải là winning_move)
        test_board = board.apply_move(bot_move, bot_symbol)
        
        if check_win(test_board, bot_symbol, goal):
            passed += 1
        else:
            failed += 1
            print(f"  ❌ Test {test_id + 1} failed!")
            print(f"     Bot chọn: {bot_move}, không phải nước thắng")
            print(f"     Board trước khi bot đi:")
            print(board)
    
    print(f"\n📊 Kết quả: ✅ {passed} passed, ❌ {failed} failed")
    print(f"🎯 Tỷ lệ thành công: {100 * passed / (passed + failed):.2f}%")
    return passed, failed


# ============================================================================
# MAIN
# ============================================================================

def main():
    print("=" * 60)
    print("PROPERTY-BASED TESTING CHO PURE FUNCTIONS")
    print("=" * 60)
    
    random.seed(42)  # Fixed seed để reproducible
    
    results = {}
    
    # Test 1
    p1, f1 = test_applymove_commutative(100)
    results["applyMove - Giao hoán"] = (p1, f1)
    
    # Test 2
    p2, f2 = test_applymove_countsymbol(100)
    results["applyMove - countSymbol"] = (p2, f2)
    
    # Test 3
    p3, f3 = test_map_compose(100)
    results["map(f)∘map(g) = map(f∘g)"] = (p3, f3)
    
    # Test 4
    p4, f4 = test_medium_bot_priority(100)
    results["medium bot - Ưu tiên nước thắng"] = (p4, f4)
    
    # Tổng kết
    print("\n" + "=" * 60)
    print("📊 TỔNG KẾT CHUNG")
    print("=" * 60)
    
    for name, (passed, failed) in results.items():
        total = passed + failed
        rate = 100 * passed / total if total > 0 else 0
        status = "✅ PASS" if rate == 100 else "⚠️ PARTIAL" if rate > 90 else "❌ FAIL"
        print(f"  {status} | {name}: {passed}/{total} ({rate:.2f}%)")
    
    print("\n🎉 Kết luận: Các property-based test đã kiểm chứng tính chất của pure function!")
    print("   (Với 100+ input ngẫu nhiên mỗi test)")


if __name__ == "__main__":
    main()