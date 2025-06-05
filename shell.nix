{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    clang
    cmake
    gnumake
    ninja
    pkg-config
    boost
  ];
}
