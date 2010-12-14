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

# Initialize model as tetrahedron
my @v = (
  fixradius [ 1, 1, 1],
  fixradius [ 1,-1,-1],
  fixradius [-1, 1,-1],
  fixradius [-1,-1, 1]
);
my @s = (
  [0,2,1],
  [0,3,2],
  [0,1,3],
  [1,2,3]
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
    my ($p0,$p1,$p2) = @{$s};
    my ($p3,$p4,$p5) = (undef, undef, undef);
    $p3 = divpoint($p0,$p1);
    $p4 = divpoint($p1,$p2);
    $p5 = divpoint($p2,$p0);
    push @ns, [$p0,$p3,$p5];
    push @ns, [$p2,$p5,$p4];
    push @ns, [$p1,$p4,$p3];
    push @ns, [$p3,$p4,$p5];
  }
  @s = @ns;
}

# Convert to edges representation
my %e = ();
my @e = ();
for (my $i = 0; $i <= $#s; $i++) {
  my @p0 = @{$s[$i]};
  my @p1 = ();
  my $b, $a = shift @p0;
  push @p0, $a;
  while (defined($b = shift@p0)) {
    push(@p1, $e{$a,$b}), $a = $b, next if defined $e{$a,$b};
    push(@p1, $e{$b,$a}), $a = $b, next if defined $e{$b,$a};
    push @e, [$a, $b];
    push @p1, $#e;
    $e{$a,$b} = $#e;
    $a = $b;
  }
  @{$s[$i]} = @p1;
}

# Print data
printf "m\n1 0 0 0\n0 1 0 0\n0 0 1 0\n0 0 0 1\n";
printf "v%d\n", $#v+1;
for (my $i = 0; $i <= $#v; $i++) {
  printf '%+8.5f %+8.5f %+8.5f'."\n",
    $v[$i]->[0], $v[$i]->[1], $v[$i]->[2];
}
printf "e%d", $#e+1;
for (my $i = 0; $i <= $#e; $i++) {
  print "\n" if ($i % 10) == 0;
  print " "  if ($i % 10) != 0;
  printf "%d %d", $e[$i]->[0], $e[$i]->[1];
}
print "\n";
printf "s%d", $#s+1;
for (my $i = 0; $i <= $#s; $i++) {
  print "\n" if ($i % 6) == 0;
  print " "  if ($i % 6) != 0;
  printf "%d", $#{$s[$i]}+1;
  for (my $j = 0; $j <= $#{$s[$i]}; $j++) {
    printf " %d", $s[$i]->[$j];
  }
}

