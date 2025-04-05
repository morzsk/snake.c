{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.gcc
    pkgs.gdb
	pkgs.gnumake
	pkgs.ncurses
	pkgs.bear
  ];
}
