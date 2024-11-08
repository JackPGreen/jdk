package org.openjdk.bench.java.util.concurrent;

import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.BenchmarkMode;
import org.openjdk.jmh.annotations.Fork;
import org.openjdk.jmh.annotations.Measurement;
import org.openjdk.jmh.annotations.Mode;
import org.openjdk.jmh.annotations.OutputTimeUnit;
import org.openjdk.jmh.annotations.Param;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.Warmup;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@State(Scope.Benchmark)
@Warmup(iterations = 5, time = 1, timeUnit = TimeUnit.SECONDS)
@Measurement(iterations = 200, time = 10, timeUnit = TimeUnit.MILLISECONDS)
@Fork(3)
public class AtomicReferenceConstructor {
    private static final String obj = AtomicReferenceConstructor.class.getName();

    @Benchmark
    public Object testConstructorDefaultExisting() {
        return new AtomicReference<Object>(null);
    }

    @Benchmark
    public Object testConstructorDefaultImplicitExisting() {
        return new AtomicReference<Object>();
    }

    @Benchmark
    public Object testConstructorDefaultCurrent() {
        return new AtomicReferenceCurrent<Object>(null);
    }

    @Benchmark
    public Object testConstructorDefaultNew() {
        return new AtomicReferenceNew<Object>(null);
    }

    @Benchmark
    public Object testConstructorPopulatedExisting() {
        return new AtomicReference<Object>(obj);
    }

    @Benchmark
    public Object testConstructorPopulatedCurrent() {
        return new AtomicReferenceCurrent<Object>(obj);
    }

    @Benchmark
    public Object testConstructorPopulatedNew() {
        return new AtomicReferenceNew<Object>(obj);
    }

    public static class AtomicReferenceCurrent<V> {
        private volatile V value;

        public AtomicReferenceCurrent(V initialValue) {
            value = initialValue;
        }
    }

    public static class AtomicReferenceNew<V> {
        private volatile V value;

        public AtomicReferenceNew(V initialValue) {
            if (initialValue != null) {
                value = initialValue;
            }
        }
    }
}
