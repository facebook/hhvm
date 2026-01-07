<?hh

class G_MultiPath {
  // Short path: root <- path1 <- join (distance 2 from root to join)
  public static function path1(): void {
    G_Root::multiPathRoot();
  }

  // Long path: root <- path2a <- path2b <- join (distance 3 from root to join)
  public static function path2a(): void {
    G_Root::multiPathRoot();
  }

  public static function path2b(): void {
    self::path2a();
  }

  // Join point: called via both paths
  public static function join(): void {
    self::path1();
    self::path2b();
  }
}
