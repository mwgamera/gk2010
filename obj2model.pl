#!/usr/bin/perl
use strict;

my @v = ();
my @s = ();

# Read objects
while (<>) {
  s/#.*$//;
  next if m/^\s*$/;
  split;
  if ($_[0] eq 'v') {
    push @v, [$_[3], -$_[1], -$_[2]];
    next;
  }
  if ($_[0] eq 'f') {
    shift @_;
    @_ = reverse map { int($_)-1 } @_;
    push @s, [@_];
    next;
  }
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

