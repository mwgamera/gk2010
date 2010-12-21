#!/usr/bin/perl
use strict;
use Math::Trig;

# Take xyz vector and set its length to one
sub fixradius($) {
  my ($x,$y,$z) = @{(shift)};
  my $t = acos($z/sqrt($x*$x+$y*$y+$z*$z));
  my $f = atan2($y,$x);
  $x = sin($t)*cos($f);
  $y = sin($t)*sin($f);
  $z = cos($t);
  return [$x,$y,$z];
}

# Initialize model as cube
my @v = (
  fixradius [-1,-1,-1],
  fixradius [ 1,-1,-1],
  fixradius [ 1, 1,-1],
  fixradius [-1, 1,-1],
  fixradius [-1,-1, 1],
  fixradius [ 1,-1, 1],
  fixradius [ 1, 1, 1],
  fixradius [-1, 1, 1]
);
my @s = (
  [3,2,1,0],
  [2,6,5,1],
  [6,7,4,5],
  [0,1,5,4],
  [4,7,3,0],
  [7,6,2,3]
);

# Subdivide
for (my $i = 0; $i < int $ARGV[0]; $i++) {
  my %ns = ();
  my @ns = ();
  sub divpoint($$) {
    my $p0 = shift;
    my $p1 = shift;
    return $ns{$p0,$p1} if defined $ns{$p0,$p1};
    return $ns{$p1,$p0} if defined $ns{$p1,$p0};
    push @v, fixradius[
        ($v[$p0]->[0]+$v[$p1]->[0]) / 2,
        ($v[$p0]->[1]+$v[$p1]->[1]) / 2,
        ($v[$p0]->[2]+$v[$p1]->[2]) / 2
      ];
    $ns{$p0,$p1} = $#v;
    return $#v;
  }
  for my$s (@s) {
    my ($p0,$p1,$p2,$p3) = @{$s};
    my ($p4,$p5,$p6,$p7,$p8) =
      (undef, undef, undef, undef, undef);
    $p4 = divpoint($p0,$p1);
    $p5 = divpoint($p1,$p2);
    $p6 = divpoint($p2,$p3);
    $p7 = divpoint($p3,$p0);
    $p8 = divpoint($p0,$p2);
    push @ns, [$p0,$p4,$p8,$p7];
    push @ns, [$p1,$p5,$p8,$p4];
    push @ns, [$p2,$p6,$p8,$p5];
    push @ns, [$p3,$p7,$p8,$p6];
  }
  @s = @ns;
}

# Print data
print "MeshVersionFormatted 1\n";
print "Dimension 3\n";
printf "Vertices %d\n", $#v+1;
for (my $i = 0; $i <= $#v; $i++) {
  printf '%+8.5f %+8.5f %+8.5f 00'."\n",
    $v[$i]->[0], $v[$i]->[1], $v[$i]->[2];
}
printf "Quadrilaterals %d\n", $#s+1;
for (my $i = 0; $i <= $#s; $i++) {
  printf "%d %d %d %d 00\n",
    1+$s[$i]->[0], 1+$s[$i]->[1],
    1+$s[$i]->[2], 1+$s[$i]->[3];
}
print "End\n";
